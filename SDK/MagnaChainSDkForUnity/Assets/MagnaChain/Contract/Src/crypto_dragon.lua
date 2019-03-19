--[[
  PersistentData = {} 持久化数据
  文件内global函数是可供外部调用的
  参数是个table,外部传入json串经过底层转换成table
  返回参数需要是一个table,底层会把它转成json串传给外部应用,注意返回table里只能包含数字、字符串和bool值。
]]

local function check_address(addr)
	return type(addr) == "string" and addr ~= ""
end

--be called by system when contract register. 
function init()
	PersistentData = {}
	PersistentData.kitties = {}
	PersistentData.kittyIndexToOwner = {} --player address to his dragon id
	PersistentData.ownershipTokenCount = {}
	PersistentData.kittyIndexToApproved = {}
	PersistentData.sireAllowedToAddress = {}
	
	PersistentData.lastGenId = 0
	PersistentData.ownerAddress = msg.sender
	--Access control
	PersistentData.ceoAddress = msg.sender --The CEO can reassign other roles and change the addresses of our dependent smart 
	                               --contracts. It is also the only role that can unpause the smart contract. It is 
								   --initially set to the address that created the smart contract in the KittyCore 
								   --constructor.
	PersistentData.cfoAddress = "" --The CFO can withdraw funds from KittyCore and its auction contracts.
	PersistentData.cooAddress = "" --The COO: The COO can release gen0 kitties to auction, and mint promo cats.
	PersistentData.paused = false  --Keeps track whether the contract is paused. When that is true, most actions are blocked

	PersistentData.secondsPerBlock = 15
	
	--KittyBreeding
	PersistentData.autoBirthFee = 100
	PersistentData.pregnantKitties = 0
	
	-- Tracks last 5 sale price of gen0 kitty sales
	PersistentData.gen0SaleCount = 0
	PersistentData.lastGen0SalePrices = {}
	
	PersistentData.ownerCut = 0 --Cut owner takes on each auction, measured in basis points (1/100 of a percent).Values 0-10,000 map to 0%-100%
	PersistentData.tokenIdToAuction = {} --token ID to their corresponding auction.
	
	PersistentData.promoCreatedCount = 0
	PersistentData.gen0CreatedCount = 0
end
-------------------------------------------------------------------------
--access control
local function onlyOwner()
	assert(PersistentData.ownerAddress == msg.sender)
end

function transferOwnership( params )
	onlyOwner()
	local _newOwner = params.newOwner
	assert(check_address(_newOwner))
	PersistentData.ownerAddress = _newOwner
end

--ERC721 interface ... Required methods, Optional, events
--ERC-165 Compatibility

--contract GeneScienceInterface

-------------------------------------------------------------------------
--KittyAccessControl
local function onlyCEO()
	assert(PersistentData.ceoAddress == msg.sender)
end

local function onlyCFO()
	assert(PersistentData.cfoAddress == msg.sender)
end

local function onlyCOO()
	assert(PersistentData.cooAddress == msg.sender)
end

local function onlyCLevel()
	assert(PersistentData.ceoAddress == msg.sender or
			PersistentData.cfoAddress == msg.sender or
			PersistentData.cooAddress == msg.sender
		);
end

function setCEO( params )
	onlyCEO()
	
	local _newCEO = params.newCEO
	assert(check_address(_newCEO))
	
	PersistentData.ceoAddress = _newCEO
end

function setCFO(params)
	onlyCEO()
	
	local _newCFO = params.newCFO
	assert(check_address(_newCFO))
	PersistentData.cfoAddress = _newCFO	
end

function setCOO(params)
	onlyCEO()
	
	local _newCOO = params.newCOO
	assert(check_address(_newCOO))
	PersistentData.cooAddress = _newCOO	
end

local function whenNotPaused()
	assert(not PersistentData.paused)
end

local function whenPaused()
	assert(PersistentData.paused)
end

function pause()
	onlyCLevel()
	whenNotPaused()
	
	PersistentData.paused = true
end

function unpause()
    --    assert(saleAuction ~= address(0));
    --    assert(siringAuction ~= address(0));
    --    assert(geneScience ~= address(0));
    --    assert(newContractAddress == address(0));

	onlyCEO()
	whenPaused()
	
	PersistentData.paused = false
end

-------------------------------------------------------------------------
--contract KittyBase 
cooldowns = {
60,
2*60,
5*60,
10*60,
30*60,
1*3600,
2*3600,
4*3600,
8*3600,
16*3600,
1*86400,
2*86400,
4*86400,
8*86400,
}

--Assigns ownership of a specific Kitty to an address.
local function _transfer( _from, _to, _tokenId )
	--Since the number of kittens is capped to 2^32 we can't overflow this
	PersistentData.ownershipTokenCount[_to] = (PersistentData.ownershipTokenCount[_to] or 0) + 1
	--transfer ownership
	PersistentData.kittyIndexToOwner[_tokenId] = _to
	--When creating new kittens _from is 0x0, but we can't account that address.
	if PersistentData.ownershipTokenCount[_from] then
		PersistentData.ownershipTokenCount[_from] = PersistentData.ownershipTokenCount[_from] - 1
	end
	--once the kitten is transferred also clear sire allowances
	PersistentData.sireAllowedToAddress[_tokenId] = nil
	--clear any previously approved ownership exchange
	PersistentData.kittyIndexToApproved[_tokenId] = nil
	--FireEvent
end

local function _createKitty( _matronId, _sireId, _generation, _genes, _owner )	
	--New kitty starts with the same cooldown as parent gen/2
	local cooldownIndex = _generation/2
	if cooldownIndex > 13 then
		cooldownIndex = 13
	end
	
	local newDragon = {
		genes = _genes, --genetic code
		birthTime = msg.blocktimestamp,
		cooldownEndBlock = 0,
		matronId = _matronId, --mother id
		sireId = _sireId, --father id
		siringWithId = 0,
		cooldownIndex = cooldownIndex, --index to cooldowns table
		generation = _generation,
	}
	
	local newKittenId = PersistentData.lastGenId + 1
	assert(newKittenId > PersistentData.lastGenId)
	PersistentData.lastGenId = newKittenId
	
	--FireEvent("CreateKitty",_owner,newKittenId)
	
	table.insert(PersistentData.kitties, newDragon)
	_transfer(0, _owner, newKittenId)
	return newKittenId
end

--Any C-level can fix how many seconds per blocks are currently observed.
function setSecondsPerBlock( params )
	onlyCLevel()
	
	local _secs = params.secs
	assert(_secs < cooldowns[1])
	PersistentData.secondsPerBlock = _secs
end

-------------------------------------------------------------------------
--ERC721Metadata
function getMetadata( params )
	local _tokenId, _preferredTransport = params.tokenId, params.preferredTransport
	
	if _tokenId == 1 then
		return {count = 15, buffer = {"Hello World! :D"}}
	elseif _tokenId == 2 then
		return {count = 49, buffer = {"I would definitely choose a medi",
									  "um length string."}}
	elseif _tokenId == 3 then
		return {count = 128, buffer = {"Lorem ipsum dolor sit amet, mi e",
                                       "st accumsan dapibus augue lorem,",
                                       " tristique vestibulum id, libero",
                                       " suscipit varius sapien aliquam.",}}
	end
end

-------------------------------------------------------------------------
--KittyOwnership
--ERC20 
function name()
	return {"CryptoDragon"}
end

function symbol()
	return {"CD"}
end
-------------------------------------------------------------------------
--KittyOwnership
--Checks if a given address is the current owner of a particular Kitty.
local function _owns( _claimant, _tokenId )
	return PersistentData.kittyIndexToOwner[_tokenId] == _claimant
end

--Checks if a given address currently has transferApproval for a particular Kitty.
local function _approvedFor( _claimant, _tokenId )
	return PersistentData.kittyIndexToApproved[_tokenId] == _claimant
end

local function _approve( _tokenId, _approved )
	PersistentData.kittyIndexToApproved[_tokenId] = _approved
end

--Returns the number of Kitties owned by a specific address.
--Required for ERC-721 compliance
function balanceOf( params )
	local _owner = params.owner
	return {count = PersistentData.ownershipTokenCount[_owner] or 0}
end

--Transfers a Kitty to another address.
--Required for ERC-721 compliance.
function transfer( params )
	whenNotPaused()
	
	local _to, _tokenId = params.to , params.tokenId--to The address of the recipient, can be a user or contract
													--tokenId The ID of the Kitty to transfer.
	assert(check_address(_to))
	assert(_to ~= msg.sender)
	assert(_to ~= msg.thisAddress)
	
	-- TODO
	--Disallow transfers to the auction contracts to prevent accidental
	--misuse. Auction contracts should only take ownership of kitties
	--through the allow + transferFrom flow.
--	assert(_to != address(saleAuction));
--	assert(_to != address(siringAuction));
	
	--You can only send your own cat.
	assert(_owns(msg.sender, _tokenId))
	_transfer(msg.sender, _to, _tokenId)
end

--Grant another address the right to transfer a specific Kitty via
--transferFrom(). This is the preferred flow for transfering NFTs to contracts.
--ERC-721 compliance.
function approve( params )
	whenNotPaused()
	
	local _to, _tokenId = params.to, params.tokenId
	
	assert(_owns(msg.sender, _tokenId))
	
	_approve(_tokenId, _to)
	
	--FireEvent
end

--Transfer a Kitty owned by another address, for which the calling address
--has previously been granted transfer approval by the owner.
--ERC-721 compliance.
function transferFrom( params )
	whenNotPaused()
	
	local _from = params.from
	local _to = params.to
	local _tokenId = params.tokenId
	
	assert(check_address(_to))
	assert(_to ~= msg.thisAddress)
	assert(_approvedFor(msg.sender, _tokenId))
	assert(_owns(_from, _tokenId))
	
	_transfer(_from, _to, _tokenId)
end

--Returns the total number of Kitties currently in existence.
--ERC-721 compliance
function totalSupply()
	return {PersistentData.lastGenId}
end

--Returns the address currently assigned ownership of a given Kitty.
--ERC-721 compliance
function ownerOf( params )
	local _tokenId = params.tokenId
	
	local owner = PersistentData.kittyIndexToOwner[_tokenId]
	
	assert(check_address(owner))
	
	return {owner = owner}
end

--Returns a list of all Kitty IDs assigned to an address.
--This method MUST NEVER be called by smart contract code. First, it's fairly
--expensive (it walks the entire Kitty array looking for cats belonging to owner),
--but it also returns a dynamic array, which is only supported for web3 calls, and
--not contract-to-contract calls.
function tokensOfOwner( params )
	local _owner = params.owner
	
	local ownerTokens = {}
	for catId, ownerAddr in pairs(PersistentData.kittyIndexToOwner) do
		if ownerAddr == _owner then
			table.insert(ownerTokens, catId)
		end
	end
	return ownerTokens
end

-------------------------------------------------------------------------
--KittyBreeding
--The minimum payment required to use breedWithAuto(). This fee goes towards
--the gas cost paid by whatever calls giveBirth(), and can be dynamically updated by
--the COO role as the gas price changes.
--autoBirthFee = 100

local function _isReadyToBreed( dragon )
	return dragon.siringWithId == 0 and dragon.cooldownEndBlock <= msg.blocknumber
end

local function _isSiringPermitted( _sireId, _matronId )
	local matronOwner = PersistentData.kittyIndexToOwner[_matronId]
	local sireOwner = PersistentData.kittyIndexToOwner[_sireId]
	
	return matronOwner == sireOwner or PersistentData.sireAllowedToAddress[_sireId] == matronOwner
end

local function _triggerCooldown( dragon )
	dragon.cooldownEndBlock = cooldowns[dragon.cooldownIndex]/PersistentData.secondsPerBlock + msg.blocknumber
	
	if dragon.cooldownIndex < 13 then
		dragon.cooldownIndex = dragon.cooldownIndex + 1
	end
end

function approveSiring( params )
	whenNotPaused()
	
	local _addr = params.addr
	local _sireId = params.sireId
	
	assert(_owns(msg.sender, _sireId))
	PersistentData.sireAllowedToAddress[_sireId] = _addr
end

function setAutoBirthFee( params )
	onlyCOO()
	
	local _val = params.val
	PersistentData.autoBirthFee = _val
end

local function _isReadyToGiveBirth( dragonMatron )
	return dragonMatron.siringWithId ~= 0 and dragonMatron.cooldownEndBlock <= msg.blocknumber
end

function isReadyToBreed( params )
	local _kittyId = params.kittyId
	
	assert(_kittyId > 0)
	local dragon = PersistentData.kitties[_kittyId]
	local b = _isReadyToBreed( dragon )
	return {b}
end

function isPregnant( params )
	local _kittyId = params.kittyId
	
	assert(_kittyId > 0)
	return {PersistentData.kitties[_kittyId].siringWithId ~= 0}
end

--Internal check to see if a given sire and matron are a valid mating pair. DOES NOT
--check ownership permissions (that is up to the caller).
--
local function _isValidMatingPair( _dragonMatron, _matronId, _dragonSire, _sireId )
	--A Kitty can't breed with itself!
	if _matronId == _sireId then
		return false
	end
	
	--Kitties can't breed with their parents.
	if _dragonMatron.matronId == _sireId or _dragonMatron.sireId == _sireId then
		return false
	end
	if _dragonSire.matronId == _matronId or _dragonSire.sireId == _matronId then
		return false
	end
	
	--We can short circuit the sibling check (below) if either cat is
	--gen zero (has a matron ID of zero).
	if _dragonSire.matronId == 0 or _dragonMatron.matronId == 0 then
		return true
	end
	
	--Kitties can't breed with full or half siblings.
	if _dragonSire.matronId == _dragonMatron.matronId or _dragonSire.matronId == _dragonMatron.sireId then
		return false
	end
	if _dragonSire.sireId == _dragonMatron.matronId or _dragonSire.sireId == _dragonMatron.sireId then
		return false
	end
	return true
end

local function _canBreedWithViaAuction( _matronId, _sireId )
	local matron = PersistentData.kitties[_matronId]
	local sire = PersistentData.kitties[_sireId]
	return _isValidMatingPair( matron, _matronId, sire, _sireId )
end

function canBreedWith( params )
	local _matronId = params.matronId
	local _sireId = params.sireId
	
	assert(_matronId > 0)
	assert(_sireId > 0)
	local matron = PersistentData.kitties[_matronId]
	local sire = PersistentData.kitties[_sireId]
	
	local bRet = _isValidMatingPair(matron, _matronId, sire, _sireId) and 
		_isSiringPermitted(_sireId, _matronId)
	return {bRet}
end

local function _breedWith( _matronId, _sireId )
	local sire = PersistentData.kitties[_sireId]
	local matron = PersistentData.kitties[_matronId]
	
	matron.siringWithId = _sireId
	
	_triggerCooldown(sire)
	_triggerCooldown(matron)
	
	--Clear siring permission for both parents. This may not be strictly necessary
	--but it's likely to avoid confusion!
	PersistentData.sireAllowedToAddress[_matronId] = nil
	PersistentData.sireAllowedToAddress[_sireId] = nil
	
	PersistentData.pregnantKitties = PersistentData.pregnantKitties + 1
	--FireEvent
end

function breedWithAuto( params )
	whenNotPaused()
	
	local _matronId = params.matronId
	local _sireId = params.sireId
	
	--Checks for payment.
	assert(msg.payment >= PersistentData.autoBirthFee)
	
	--Caller must own the matron.
	assert(_owns(msg.sender, _matronId))
	
	-- Neither sire nor matron are allowed to be on auction during a normal
	-- breeding operation, but we don't need to check that explicitly.
	-- For matron: The caller of this function can't be the owner of the matron
	--   because the owner of a Kitty on auction is the auction house, and the
	--   auction house will never call breedWith().
	-- For sire: Similarly, a sire on auction will be owned by the auction house
	--   and the act of transferring ownership will have cleared any oustanding
	--   siring approval.
	-- Thus we don't need to spend gas explicitly checking to see if either cat
	-- is on auction.

	-- Check that matron and sire are both owned by caller, or that the sire
	-- has given siring permission to caller (i.e. matron's owner).
	-- Will fail for _sireId = 0
	assert(_isSiringPermitted(_sireId, _matronId))
	
	--Grab a reference to the potential matron
	local matron = PersistentData.kitties[_matronId]
	
	--Make sure matron isn't pregnant, or in the middle of a siring cooldown
	assert(_isReadyToBreed(matron))
	
	--Grab a reference to the potential sire
	local sire = PersistentData.kitties[_sireId]
	
	--Make sure sire isn't pregnant, or in the middle of a siring cooldown
	assert(_isReadyToBreed(sire))
	
	--Test that these cats are a valid mating pair.
	assert(_isValidMatingPair(
            matron,
            _matronId,
            sire,
            _sireId
        ))
		
	--All checks passed, kitty gets pregnant!
	_breedWith(_matronId, _sireId)
end

function giveBirth( params )
	whenNotPaused()
	
	local _matronId = params.matronId
	--Grab a reference to the matron in storage.
	local matron = PersistentData.kitties[_matronId]
	--Check that the matron is a valid cat.
	assert(matron.birthTime ~= 0)
	--Check that the matron is pregnant, and that its time has come!
	assert(_isReadyToGiveBirth(matron))
	
	--Grab a reference to the sire in storage.
	local sireId = matron.siringWithId
	local sire = PersistentData.kitties[sireId]
	
	--Determine the higher generation number of the two parents
	local parentGen = math.max(matron.generation, sire.generation)
	
	--Call the sooper-sekret gene mixing operation.
	local childGenes = matron.genes + sire.genes + (matron.cooldownEndBlock - 1) --TODO geneScience.mixGenes
	
	--Make the new kitten!
	local owner = PersistentData.kittyIndexToOwner[_matronId]
	local dragonId = _createKitty(_matronId, matron.siringWithId, parentGen + 1, childGenes, owner)
	
	--Clear the reference to sire from the matron (REQUIRED! Having siringWithId
	--set is what marks a matron as being pregnant.)
	matron.siringWithId = 0
	
	--Every time a kitty gives birth counter is decremented.
	PersistentData.pregnantKitties = PersistentData.pregnantKitties - 1
	
	--TODO Send the balance fee to the person who made birth happen.
--	msg.sender.transfer(PersistentData.autoBirthFee)
	
	return {dragonId = dragonId}
end

-------------------------------------------------------------------------
--get data by page
function getDragonData( params )
	local _pagesize = params and params.pagesize
	local _pageindex = params and params.pageindex
    if _pagesize and _pageindex then
		if _pagesize <= 0 then
            _pagesize = 15
        end
        local size = #PersistentData.kitties
        local totalpage = math.ceil(size/_pagesize)
        local pagestart = math.max(_pageindex-1, 0)
		if pagestart >= totalpage and totalpage > 1 then
            pagestart = totalpage - 1
        end
        local endindex = math.min((pagestart+1)*_pagesize,size)
        local pagedata = {}
		local startindex = pagestart*_pagesize+1
        for i=startindex, endindex do
            table.insert(pagedata, PersistentData.kitties[i])
        end
        return pagedata
    end
	return PersistentData.kitties
end

-------------------------------------------------------------------------
--Auction Core
--contract ClockAuctionBase

--Escrows the NFT, assigning ownership to this contract.
--Throws if the escrow fails.
local function _escrow(_owner, _tokenId)
	local auctionAddr = msg.thisAddress
	transferFrom({from = _owner, to = auctionAddr, tokenId = _tokenId})
end

local function _addAuction(_tokenId, _auction)
	assert(_auction.duration >= 60)
	PersistentData.tokenIdToAuction[_tokenId] = _auction
	
	--FireEvent
end

local function _createAuction(_tokenId, _startingPrice, _endingPrice, _duration, _seller)
	assert(_owns(msg.sender, _tokenId))
	
	local _auction = {}
	--Current owner of NFT
	_auction.seller = _seller
	--Price (in wei) at beginning of auction
	_auction.startingPrice = _startingPrice
	--Price (in wei) at end of auction
	_auction.endingPrice = _endingPrice
	--Duration (in seconds) of auction
	_auction.duration = _duration
	--Time when auction started
    -- NOTE: 0 if this auction has been concluded
	_auction.startedAt = msg.blocktimestamp
	
	_addAuction(_tokenId, _auction)
	return _auction
end

local function _removeAuction(_tokenId)
	PersistentData.tokenIdToAuction[_tokenId] = nil
end

local function _isOnAuction(_auction)
	return _auction.startedAt > 0
end

local function _cancelAuction(_tokenId, _seller)
	_removeAuction(_tokenId)
	--_transfer(_seller, _tokenId);
	--FireEvent
end

function cancelAuction(params)
	local _tokenId = params.tokenId
	
	local auction  = PersistentData.tokenIdToAuction[_tokenId]
	assert(_isOnAuction(auction))
	assert(msg.sender == auction.seller)
	
	_cancelAuction(_tokenId, _seller)
end

function cancelAuctionWhenPaused(params)
	local _tokenId = params.tokenId
	
	local auction  = PersistentData.tokenIdToAuction[_tokenId]
	assert(_isOnAuction(auction))
	_cancelAuction(_tokenId, auction.seller);
end

local function _computeCurrentPrice(_startingPrice, _endingPrice, _duration, _secondsPassed)
	if _secondsPassed >= _duration then
		return _endingPrice
	end
	local totalPriceChange = _endingPrice - _startingPrice
	local currentPriceChange = totalPriceChange*_secondsPassed/_duration
	local currentPrice = _startingPrice + currentPriceChange
	return currentPrice
end

local function _currentPrice(_auction)
	local secondsPassed = 0
	
	if msg.blocktimestamp > _auction.startedAt then
		secondsPassed = msg.blocktimestamp - _auction.startedAt
	end
	
	return _computeCurrentPrice(
		_auction.startingPrice,
		_auction.endingPrice,
		_auction.duration,
		secondsPassed)
end

local function _computeCut(_price)
	return _price*PersistentData.ownerCut/10000
end

--Computes the price and transfers winnings.
--Does NOT transfer ownership of token.
local function _bid( _tokenId, _bidAmount)
	local auction = PersistentData.tokenIdToAuction[_tokenId]
	
	-- Explicitly check that this auction is currently live.
	-- (Because of how Ethereum mappings work, we can't just count
	-- on the lookup above failing. An invalid _tokenId will just
	-- return an auction object that is all zeros.)
	assert(_isOnAuction(auction))
	
	--Check that the bid is greater than or equal to the current price
	local price = _currentPrice(auction)
	assert(_bidAmount >= price)
	
	--Grab a reference to the seller before the auction struct
	--gets deleted.
	local seller = auction.seller
	
	--The bid is good! Remove the auction before sending the fees
	--to the sender so we can't have a reentrancy attack.
	_removeAuction(_tokenId)
	
	--Transfer proceeds to seller (if there are any!)
	if price > 0 then
		-- Calculate the auctioneer's cut.
		-- (NOTE: _computeCut() is guaranteed to return a
		-- value <= price, so this subtraction can't go negative.)
		local auctioneerCut = _computeCut(price)
		local sellerProceeds = price - auctioneerCut
		
		--seller.transfer(sellerProceeds); TODO: address transfer
	end
	
	-- Calculate any excess funds included with the bid. If the excess
	-- is anything worth worrying about, transfer it back to bidder.
	-- NOTE: We checked above that the bid amount is greater than or
	-- equal to the price so this cannot underflow.
	local bidExcess = _bidAmount - price
	
	--msg.sender.transfer(bidExcess); TODO:
	--FireEvent
	
	return price
end

-------------------------------------------------------------------------
--contract ClockAuction

function withdrawBalance()
	assert(msg.sender == PersistentData.ownerAddr)
	
	--We are using this boolean method to make sure that even if one fails it will still work
	--TODO contract address transfer
end

function getAuction(params)
	local _tokenId = params.tokenId
	
	local auction  = PersistentData.tokenIdToAuction[_tokenId]
	assert(_isOnAuction(auction))
	return {
		seller = auction.seller,
		startingPrice = auction.startingPrice,
		endingPrice = auction.endingPrice,
		duration = auction.duration,
		startedAt = auction.startedAt,
	}
end

function getCurrentPrice(params)
	local _tokenId = params.tokenId
	
	local auction  = PersistentData.tokenIdToAuction[_tokenId]
	assert(_isOnAuction(auction))
	return {price = _currentPrice(auction)}
end

-------------------------------------------------------------------------

function createAuction(params)
	whenNotPaused()
	
	local _tokenId = params.tokenId
	local _startingPrice = params.startingPrice
	local _endingPrice = params.endingPrice
	local _duration = params.duration
	local _seller = msg.sender --Seller, if not the message sender?

	local _auction = _createAuction(_tokenId, _startingPrice, _endingPrice, _duration, _seller)
	return _auction
end

--contract ClockAuction
--contract SiringClockAuction
--contract SaleClockAuction
function bid( params )
	local _tokenId = params.tokenId
	
	local seller = PersistentData.kitties[_tokenId].seller
	local price = _bid(_tokenId, msg.payment)
	_transfer(msg.sender, _tokenId)
	
	if seller == msg.thisAddress then
		local index = PersistentData.gen0SaleCount % 5 + 1
		PersistentData.lastGen0SalePrices[index] = price
		PersistentData.gen0SaleCount = index
	end
end

function averageGen0SalePrice()
	local sum = 0
	for i,v in pairs(PersistentData.lastGen0SalePrices) do
		sum = sum + v
	end
	return sum/5
end

-------------------------------------------------------------------------
--contract KittyAuction is KittyBreeding 
--setSaleAuctionAddress
--setSiringAuctionAddress
--createSaleAuction
--createSiringAuction


PROMO_CREATION_LIMIT = 5000

GEN0_CREATION_LIMIT = 45000

GEN0_STARTING_PRICE = 10 * math.pow(10,6)

GEN0_AUCTION_DURATION = 86400

-- we can create promo kittens, up to a limit. Only callable by COO
function createPromoKitty(params)
	onlyCOO()
	
	local _genes, _owner = params.genes, params.owner
	
	local kittyOwner = _owner
	if not check_address(kittyOwner) then
		kittyOwner = PersistentData.cooAddress
	end
	
	PersistentData.promoCreatedCount = PersistentData.promoCreatedCount + 1
	local dragonId = _createKitty(0, 0, 0, _genes, kittyOwner)
	return {dragonId = dragonId}
end

local function _computeNextGen0Price()
	local avePrice = averageGen0SalePrice()
	local nextPrice = avePrice + avePrice/2
	if nextPrice < GEN0_STARTING_PRICE then
		nextPrice = GEN0_STARTING_PRICE
	end
	return nextPrice
end

--Creates a new gen0 kitty with the given genes and
--creates an auction for it.
function createGen0Auction(params)
	onlyCOO()
	
	local _genes = params.genes
	assert(PersistentData.gen0CreatedCount < GEN0_CREATION_LIMIT)
	local kittyId = _createKitty(0, 0, 0, _genes, msg.thisAddress)
	--_approve(kittyId, saleAuction)
	
	_createAuction(kittyId, _computeNextGen0Price(), 0, GEN0_AUCTION_DURATION, msg.thisAddress) --to address(this)
	
	PersistentData.gen0CreatedCount = PersistentData.gen0CreatedCount + 1
end

function getKitty(params)
	local _id = params.id
	
	local kit = PersistentData.kitties[_id]
	return {
		isGestating = kit.siringWithId ~= 0,
		isReady = kit.cooldownEndBlock <= msg.blocknumber,
		cooldownIndex = kit.cooldownIndex,
		nextActionAt = kit.cooldownEndBlock,
		siringWithId = kit.siringWithId,
		birthTime = kit.birthTime,
		matronId = kit.matronId,
		sireId = kit.sireId,
		generation = kit.generation,
		genes = kit.genes,
	}
end





