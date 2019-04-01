--[[
   ERC721 interfaces:
   1- name:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT name
   2- symbol:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT symbol
   3- totalSupply:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT totalSupply

   4- exists:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT exists 1
   5- tokenOfOwnerByIndex:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT tokenOfOwnerByIndex XLCq2gvPccrv131zwZs8KHEKmXF2whQChT
   6- tokenByIndex:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT tokenByIndex 1
   7- balanceOf:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT balanceOf XLCq2gvPccrv131zwZs8KHEKmXF2whQChT
   8- ownerOf:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT ownerOf 1

   9- setApprovalForAll:
   		./cell-cli.exe sendcall XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT setApprovalForAll XLCq2gvPccrv131zwZs8KHEKmXF2whQCh1 true
   10- isApprovedForAll:
   		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT isApprovedForAll XLCq2gvPccrv131zwZs8KHEKmXF2whQChT XLCq2gvPccrv131zwZs8KHEKmXF2whQCh1
   11- approve:
		./cell-cli.exe sendcall XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT approve  XLCq2gvPccrv131zwZs8KHEKmXF2whQCh1 1
   12- getApproved:   		
		./cell-cli.exe call XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT getApproved  1
   13 - transferFrom:
   		./cell-cli.exe sendcall XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT transferFrom  XLCq2gvPccrv131zwZs8KHEKmXF2whQChT XLCq2gvPccrv131zwZs8KHEKmXF2whQCh1 1
   14 - transfer:
   		./cell-cli.exe sendcall XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT transfer XLCq2gvPccrv131zwZs8KHEKmXF2whQCh1 1


   --- test interface:
   15 - createToken:
		./cell-cli.exe sendcall XGifHfGQFNLAVfYBoD7nVjv1sD9AbyYug9 1 XLCq2gvPccrv131zwZs8KHEKmXF2whQChT createToken XLCq2gvPccrv131zwZs8KHEKmXF2whQChT 
--]]


local function check_address(addr)
	return type(addr) == "string" and addr ~= ""
end


--be called by system when contract register. 
function init()
	PersistentData = {}
	PersistentData.name = "cell-link"
	PersistentData.symbol = "$" --token symbol

	-- token
	-- Mapping from owner to list of owned token IDs
	PersistentData.ownedTokens = {}			-- mapping(address => uint256[])
	-- Array with all token ids, used for enumeration
	PersistentData.allTokens = {}			-- uint256[]
	-- Mapping from token ID to index of the owner tokens list
	PersistentData.ownedTokensIndex = {} 		-- mapping(uint256 => uint256)
	-- Mapping from token id to position in the allTokens array
    PersistentData.allTokensIndex = {}			-- mapping(uint256 => uint256)
    -- Optional mapping for token URIs
	PersistentData.tokenURIs = {}			-- mapping(uint256 => string)

	-- base token 
	-- Mapping from token ID to owner
	PersistentData.tokenOwner = {}			-- mapping (uint256 => address) 
	-- Mapping from token ID to approved address
	PersistentData.tokenApprovals = {}		-- mapping (uint256 => address)
	-- Mapping from owner to number of owned token
	PersistentData.ownedTokensCount = {} 	-- mapping (address => uint256)
	-- Mapping from owner to operator approvals
	PersistentData.operatorApprovals = {}	-- mapping (address => mapping (address => bool))

end

--[[
-- @dev Gets the token name
-- @return string representing the token name
--]]
function name()
	return PersistentData.name
end

--[[
-- @dev Gets the token symbol
-- @return string representing the token symbol
--]]
function symbol()
	return PersistentData.symbol
end

--[[
-- @dev Gets the total amount of tokens stored by the contract
-- @return uint256 representing the total amount of tokens
--]]
function totalSupply()
	return #PersistentData.allTokens
end

--[[
   * @dev Returns whether the specified token exists
   * @param _tokenId uint256 ID of the token to query the existence of
   * @return whether the token exists
 --]]
function exists(_tokenId)
	_tokenId = tonumber(_tokenId)
	local tokenOwner = PersistentData.tokenOwner
	local _owner = tokenOwner[_tokenId]
	return check_address(_owner)
end

--[[
-- @dev Returns an URI for a given token ID
-- @dev Throws if the token ID does not exist. May return an empty string.
-- @param _tokenId uint256 ID of the token to query
 --]]
function tokenURI(_tokenId)
	_tokenId = tonumber(_tokenId)
	assert(exists(_tokenId))
    return PersistentData.tokenURIs[_tokenId]
end

--[[
   * @dev Gets the token ID at a given index of the tokens list of the requested owner
   * @param _owner address owning the tokens list to be accessed
   * @param _index uint256 representing the index to be accessed of the requested tokens list
   * @return uint256 token ID at the given index of the tokens list owned by the requested address
--]]
function tokenOfOwnerByIndex(_owner, _index)
	_index = tonumber(_index)
	assert(_index <= balanceOf(_owner))
	local ownedTokens = PersistentData.ownedTokens
	if not ownedTokens[_owner] then
		return nil
	end
    return ownedTokens[_owner][_index]
end

--[[
   * @dev Gets the token ID at a given index of all the tokens in this contract
   * @dev Reverts if the index is greater or equal to the total number of tokens
   * @param _index uint256 representing the index to be accessed of the tokens list
   * @return uint256 token ID at the given index of the tokens list
--]]
function tokenByIndex(_index)
	_index = tonumber(_index)
	assert(_index <= totalSupply())
	local allTokens = PersistentData.allTokens
	return allTokens[_index]
end

--[[
   * @dev Gets the balance of the specified address
   * @param _owner address to query the balance of
   * @return uint256 representing the amount owned by the passed address
--]]
function balanceOf(_owner)
	assert(check_address(_owner))
	local ownedTokensCount = PersistentData.ownedTokensCount
    return ownedTokensCount[_owner] or 0
end

--[[
   * @dev Gets the owner of the specified token ID
   * @param _tokenId uint256 ID of the token to query the owner of
   * @return owner address currently marked as the owner of the given token ID
--]]
function ownerOf(_tokenId)
	_tokenId = tonumber(_tokenId)
	local tokenOwner = PersistentData.tokenOwner
	local _owner = tokenOwner[_tokenId]
	print("====== owner:", _owner)
   	assert(_owner and check_address(_owner))
    return _owner
end

--[[
   * @dev Sets or unsets the approval of a given operator
   * @dev An operator is allowed to transfer all tokens of the sender on their behalf
   * @param _to operator address to set the approval
   * @param _approved representing the status of the approval to be set
--]]
function setApprovalForAll(_to, _approved)
	assert(_to ~= msg.sender)
	assert(_approved == "true")
	if not PersistentData.operatorApprovals[msg.sender] then
		PersistentData.operatorApprovals[msg.sender] = {}
	end
	PersistentData.operatorApprovals[msg.sender][_to] = true
end


--[[
   * @dev Tells whether an operator is approved by a given owner
   * @param _owner owner address which you want to query the approval of
   * @param _operator operator address which you want to query the approval of
   * @return bool whether the given operator is approved by the given owner
--]]
function isApprovedForAll(_owner, _operator)
	local operatorApprovals = PersistentData.operatorApprovals
	if not operatorApprovals[_owner] then
		return false
	end
	return operatorApprovals[_owner][_operator] or false
end


--[[
   * @dev Gets the approved address for a token ID, or zero if no address set
   * @param _tokenId uint256 ID of the token to query the approval of
   * @return address currently approved for the given token ID
--]]
function getApproved(_tokenId)
	_tokenId = tonumber(_tokenId)
   return PersistentData.tokenApprovals[_tokenId]
end

--[[
   * @dev Approves another address to transfer the given token ID
   * @dev The zero address indicates there is no approved address.
   * @dev There can only be one approved address per token at a given time.
   * @dev Can only be called by the token owner or an approved operator.
   * @param _to address to be approved for the given token ID
   * @param _tokenId uint256 ID of the token to be approved
--]]
function approve(_to, _tokenId)
	_tokenId = tonumber(_tokenId)
	local _owner = ownerOf(_tokenId)
    assert(_to ~= _owner)
    assert(msg.sender == _owner or isApprovedForAll(_owner, msg.sender))

    if check_address(getApproved(_tokenId)) or check_address(_to) then
      PersistentData.tokenApprovals[_tokenId] = _to
   	end
   	return "approved", getApproved(_tokenId), "owner", ownerOf(_tokenId)
end


--[[
   * @dev Internal function to clear current approval of a given token ID
   * @dev Reverts if the given address is not indeed the owner of the token
   * @param _owner owner of the token
   * @param _tokenId uint256 ID of the token to be transferred
--]]
local function clearApproval(_owner, _tokenId)
	assert(ownerOf(_tokenId) == _owner)
	local tokenApprovals = PersistentData.tokenApprovals
	if check_address(tokenApprovals[_tokenId]) then
	  tokenApprovals[_tokenId] = nil
	end
end

--[[
   * @dev Internal function to add a token ID to the list of a given address
   * @param _to address representing the new owner of the given token ID
   * @param _tokenId uint256 ID of the token to be added to the tokens list of the given address
--]]
local function addTokenTo(_to, _tokenId)
	local tokenOwner = PersistentData.tokenOwner
    assert(not tokenOwner[_tokenId])
    tokenOwner[_tokenId] = _to
    local ownedTokensCount = PersistentData.ownedTokensCount
    -- print("=======1:", ownedTokensCount[_to], type(ownedTokensCount), type(_to))
    ownedTokensCount[_to] = (ownedTokensCount[_to] or 0) + 1
	-- print("=======2:", ownedTokensCount[_to], tokenOwner[_tokenId])

    local ownedTokens = PersistentData.ownedTokens
    local ownedTokenList = ownedTokens[_to]
    if not ownedTokenList then
    	ownedTokens[_to] = {}
    	ownedTokenList = ownedTokens[_to]
    end

    ownedTokenList[#ownedTokenList + 1] = _tokenId
    local ownedTokensIndex = PersistentData.ownedTokensIndex
    ownedTokensIndex[_tokenId] = #ownedTokenList
end


--[[
   * @dev Internal function to remove a token ID from the list of a given address
   * @param _from address representing the previous owner of the given token ID
   * @param _tokenId uint256 ID of the token to be removed from the tokens list of the given address
--]]
local function removeTokenFrom(_from, _tokenId)
    assert(ownerOf(_tokenId) == _from)
    local ownedTokensCount = PersistentData.ownedTokensCount
    ownedTokensCount[_from] = ownedTokensCount[_from] - 1 
    PersistentData.tokenOwner[_tokenId] = nil

    local ownedTokensIndex = PersistentData.ownedTokensIndex
    local tokenIndex = ownedTokensIndex[_tokenId]
    local ownedTokens = PersistentData.ownedTokens
    local lastTokenIndex = #ownedTokens[_from]
    local lastToken = ownedTokens[_from][lastTokenIndex]

    ownedTokens[_from][tokenIndex] = lastToken
    table.remove(ownedTokens[_from], lastTokenIndex)
    ownedTokensIndex[_tokenId] = nil
    ownedTokensIndex[lastToken] = tokenIndex
end



--[[
   * @dev Returns whether the given spender can transfer a given token ID
   * @param _spender address of the spender to query
   * @param _tokenId uint256 ID of the token to be transferred
   * @return bool whether the msg.sender is approved for the given token ID,
   *  is an operator of the owner, or is the owner of the token
 --]]
local function isApprovedOrOwner(_spender, _tokenId)
    local owner = ownerOf(_tokenId)
    -- print("==== isApprovedOrOwner ===", owner, _spender, _tokenId)
    return _spender == owner or getApproved(_tokenId) == _spender or isApprovedForAll(owner, _spender)
end

--[[
   * @dev Checks msg.sender can transfer a token, by being owner, approved, or operator
   * @param _tokenId uint256 ID of the token to validate
--]]
local function canTransfer(_tokenId)
    return isApprovedOrOwner(msg.sender, _tokenId)
end


--[[
   * @dev Transfers the ownership of a given token ID to another address
   * @dev Usage of this method is discouraged, use `safeTransferFrom` whenever possible
   * @dev Requires the msg sender to be the owner, approved, or operator
   * @param _from current owner of the token
   * @param _to address to receive the ownership of the given token ID
   * @param _tokenId uint256 ID of the token to be transferred
--]]
function transferFrom(_from, _to, _tokenId)
	assert(check_address(_from))
	assert(check_address(_to))

	_tokenId = tonumber(_tokenId)
	assert(canTransfer(_tokenId))
	clearApproval(_from, _tokenId)
	removeTokenFrom(_from, _tokenId)
	addTokenTo(_to, _tokenId)
	return _tokenId, "owner", ownerOf(_tokenId)
end

function transfer(_to, _tokenId)
	--print("==== tranfer ===", _to, msg.sender)
	assert(_to ~= msg.sender)
	_tokenId = tonumber(_tokenId)
	return transferFrom(msg.sender, _to, _tokenId)
end

--[[
   * @dev Internal function to mint a new token
   * @dev Reverts if the given token ID already exists
   * @param _to address the beneficiary that will own the minted token
   * @param _tokenId uint256 ID of the token to be minted by the msg.sender
 --]]
local function _mint(_to, _tokenId)
	assert(_to ~= nil)
    addTokenTo(_to, _tokenId)

    local allTokens = PersistentData.allTokens
    local allTokensIndex = PersistentData.allTokensIndex
	allTokens[#allTokens + 1] = _tokenId
	allTokensIndex[_tokenId] = #allTokens
end

function mint(_to, _tokenId)
	_tokenId = tonumber(_tokenId)
	_mint(_to, _tokenId)
end

--[[
   * @dev Internal function to burn a specific token
   * @dev Reverts if the token does not exist
   * @param _owner owner of the token to burn
   * @param _tokenId uint256 ID of the token being burned by the msg.sender
--]]
local function _burn(_owner, _tokenId)
	clearApproval(_owner, _tokenId)
    removeTokenFrom(_owner, _tokenId)

    --[[
	// Clear metadata (if any)
	if (bytes(tokenURIs[_tokenId]).length != 0) {
	  delete tokenURIs[_tokenId];
	}
	--]]

	-- Reorg all tokens array
	local allTokensIndex = PersistentData.allTokensIndex
	local tokenIndex = allTokensIndex[_tokenId]

	local allTokens = PersistentData.allTokens
	local lastTokenIndex = #allTokens
	local lastToken = allTokens[lastTokenIndex]

	allTokens[tokenIndex] = lastToken
	table.remove(allTokens, lastTokenIndex)

	allTokensIndex[_tokenId] = nil
	allTokensIndex[lastToken] = tokenIndex
end

function burn(_owner, _tokenId)
	_tokenId = tonumber(_tokenId)
	_burn(_owner, _tokenId)
end


--[[
   * @dev Internal function to set the token URI for a given token
   * @dev Reverts if the token ID does not exist
   * @param _tokenId uint256 ID of the token to set its URI
   * @param _uri string URI to assign
 --]]
local function _setTokenURI(_tokenId, _uri)
    assert(exists(_tokenId))
    tokenURIs[_tokenId] = _uri
end

function setTokenURI(_tokenId, _uri)
	_tokenId = tonumber(_tokenId)
    _setTokenURI(_tokenId, _uri)
end

--test interface
function createToken(_to)
	local tokenId = totalSupply() + 1
	-- print("===== createToken: ", _to, tokenId)
	_mint(_to, tokenId)
	--transfer(_to, tokenId)
	return tokenId, "owner", ownerOf(tokenId)
end