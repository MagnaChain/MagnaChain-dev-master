
function init()
	PersistentData = {}
	PersistentData.name = "token_name"
	PersistentData.symbol = "¢" --token symbol
	PersistentData.decimals = 0 --
	PersistentData.decimalsNum = math.pow(10, PersistentData.decimals)
	local initialSupply = 21000000
	PersistentData.totalSupply = initialSupply * PersistentData.decimalsNum
	PersistentData.balanceOf = {}
	PersistentData.balanceOf[msg.sender] = PersistentData.totalSupply
	PersistentData.allowance = {}
end

local function _transfer(_from, _to, _value)
	local balanceOf = PersistentData.balanceOf
	assert(_to ~= 0x0)
	_value = tonumber(_value)
	
	balanceOf[_from] = balanceOf[_from] or 0
	balanceOf[_to] = balanceOf[_to] or 0
	assert(balanceOf[_from] >= _value)
	assert(balanceOf[_to] + _value > balanceOf[_to])
	local previousBalances = balanceOf[_from] + balanceOf[_to]
	balanceOf[_from] = balanceOf[_from] - _value
	balanceOf[_to] = balanceOf[_to] + _value
	assert(balanceOf[_from] + balanceOf[_to] == previousBalances)
	if balanceOf[_from] == 0 then
		balanceOf[_from] = nil
	end
end

--[[
 Transfer tokens
 Send `_value` tokens to `_to` from your account
 @param _to The address of the recipient
 @param _value the amount to send
]]
function transfer(_to, _value)

	_transfer(msg.sender, _to, _value)
	print("transfer",msg.sender, _to, _value)
	return true
end

--[[
 Transfer tokens from other address
 Send `_value` tokens to `_to` on behalf of `_from`
 @param _from The address of the sender
 @param _to The address of the recipient
 @param _value the amount to send
]]
function transferFrom(_from, _to, _value)
	_value = tonumber(_value)
	local allowance = PersistentData.allowance
	assert(_value <= allowance[_from][msg.sender])
	allowance[_from][msg.sender] = allowance[_from][msg.sender] - _value;
    _transfer(_from, _to, _value);
    return true
end

--[[
 Set allowance for other address
 Allows `_spender` to spend no more than `_value` tokens on your behalf
 @param _spender The address authorized to spend
 @param _value the max amount they can spend
]]
function approve(_spender, _value)
	_value = tonumber(_value)
	local allowance = PersistentData.allowance
	allowance[msg.sender] = allowance[msg.sender] or {}
	allowance[msg.sender][_spender] = _value
	return true
end

--[[
 Set allowance for other address and notify
 Allows `_spender` to spend no more than `_value` tokens on your behalf, and then ping the contract about it
 @param _spender The address authorized to spend
 @param _value the max amount they can spend
 @param _extraData some extra information to send to the approved contract
]]
--[[
function approveAndCall(address _spender, uint256 _value, bytes _extraData)
	tokenRecipient spender = tokenRecipient(_spender);
	if (approve(_spender, _value)) then
		spender.receiveApproval(msg.sender, _value, this, _extraData);
		return true;
	end
end
]]

--[[
 Destroy tokens
 Remove `_value` tokens from the system irreversibly
 @param _value the amount of money to burn
]]
function burn(_value)

	_value = tonumber(_value)
	local balanceOf = PersistentData.balanceOf
    assert(balanceOf[msg.sender] >= _value)  						 -- Check if the sender has enough
	balanceOf[msg.sender] = balanceOf[msg.sender] - _value           -- Subtract from the sender
	PersistentData.totalSupply = PersistentData.totalSupply - _value -- Updates totalSupply
	return true
end

--[[
 Destroy tokens from other account
 Remove `_value` tokens from the system irreversibly on behalf of `_from`.
 @param _from the address of the sender
 @param _value the amount of money to burn
]]
function burnFrom(_from, _value)
	_value = tonumber(_value)
	local balanceOf = PersistentData.balanceOf
	local allowance = PersistentData.allowance
	
	balanceOf[_from] = balanceOf[_from] or 0
	
	assert(balanceOf[_from] >= _value);                	-- Check if the targeted balance is enough
	assert(_value <= allowance[_from][msg.sender]);    	-- Check allowance
	balanceOf[_from] = balanceOf[_from] - _value;		-- Subtract from the targeted balance
	allowance[_from][msg.sender] = allowance[_from][msg.sender] - _value;-- Subtract from the sender's allowance
	PersistentData.totalSupply = PersistentData.totalSupply - _value;	 -- Update totalSupply
	return true
end

function name()
	return PersistentData.name
end

function symbol()
	return PersistentData.symbol
end

function decimals()
	return PersistentData.decimals
end

function totalSupply()
	return PersistentData.totalSupply
end

function getBalanceOf(_whoaddr)
	local addr = _whoaddr or msg.sender
	
	local balanceOf = PersistentData.balanceOf
	local balance = balanceOf[addr] or 0
	return addr, balance
end
