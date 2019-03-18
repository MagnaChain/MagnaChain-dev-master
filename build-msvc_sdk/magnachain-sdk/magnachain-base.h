#pragma once

#include <vector>


class CellKeyID;

//class uint256;
template<unsigned int BITS>
class base_blob
{
protected:
	enum { WIDTH = BITS / 8 };
	uint8_t data[WIDTH];
public:
	base_blob()
	{
		memset(data, 0, sizeof(data));
	}
};

class uint256 : public base_blob<256>
{
public:
	uint256() {}
	uint256(const base_blob<256>& b) : base_blob<256>(b) {}	
};

typedef uint256 ChainCode;

//template <typename T>
//struct secure_allocator : public std::allocator<T>;

//typedef std::vector<unsigned char, secure_allocator<unsigned char> > CPrivKey;

class CellPubKey
{
public:
	//! Construct an invalid public key.
	CellPubKey();	

	//! Initialize a public key using begin/end iterators to byte data.
	template <typename T>
	void Set(const T pbegin, const T pend);
	
	//! Construct a public key using begin/end iterators to byte data.
	template <typename T>
	CellPubKey(const T pbegin, const T pend);	

	//! Construct a public key from a byte vector.
	CellPubKey(const std::vector<unsigned char>& _vch);

	//! Simple read-only vector-like interface to the pubkey data.
	unsigned int size() const;
	const unsigned char* begin() const;
	const unsigned char* end() const;
	const unsigned char& operator[](unsigned int pos) const;

	//! Comparator implementation.
	friend bool operator==(const CellPubKey& a, const CellPubKey& b);
	
	friend bool operator!=(const CellPubKey& a, const CellPubKey& b);
	
	friend bool operator<(const CellPubKey& a, const CellPubKey& b);

	//! Implement serialization, as if this was a byte vector.
	template <typename Stream>
	void Serialize(Stream& s) const;
	
	template <typename Stream>
	void Unserialize(Stream& s);	

	//! Get the KeyID of this public key (hash of its serialization)
	CellKeyID GetID() const;

	//! Get the 256-bit hash of this public key.
	uint256 GetHash() const;

	/*
	* Check syntactic correctness.
	*
	* Note that this is consensus critical as CheckSig() calls it!
	*/
	bool IsValid() const;

	//! fully validate whether this is a valid public key (more expensive than IsValid())
	bool IsFullyValid() const;

	//! Check whether this is a compressed public key.
	bool IsCompressed() const;
	
	/**
	* Verify a DER signature (~72 bytes).
	* If this public key is not fully valid, the return value will be false.
	*/
	bool Verify(const uint256& hash, const std::vector<unsigned char>& vchSig) const;

	/**
	* Check whether a signature is normalized (lower-S).
	*/
	static bool CheckLowS(const std::vector<unsigned char>& vchSig);

	//! Recover a public key from a compact signature.
	bool RecoverCompact(const uint256& hash, const std::vector<unsigned char>& vchSig);

	//! Turn this public key into an uncompressed public key.
	bool Decompress();

	//! Derive BIP32 child pubkey.
	bool Derive(CellPubKey& pubkeyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;
};

class CellKey
{
public:
	//! Construct an invalid private key.	
	
	friend bool operator==(const CellKey& a, const CellKey& b);

	//! Initialize using begin and end iterators to byte data.
	template <typename T>
	void Set(const T pbegin, const T pend, bool fCompressedIn);	

	//! Simple read-only vector-like interface.
	unsigned int size() const;
	const unsigned char* begin() const;
	const unsigned char* end() const;

	//! Check whether this private key is valid.
	bool IsValid() const;

	//! Check whether the public key corresponding to this private key is (to be) compressed.
	bool IsCompressed() const;

	//! Generate a new private key using a cryptographic PRNG.
	void MakeNewKey(bool fCompressed);

	/**
	* Convert the private key to a CPrivKey (serialized OpenSSL private key data).
	* This is expensive.
	*/
	//CPrivKey GetPrivKey() const;

	/**
	* Compute the public key from a private key.
	* This is expensive.
	*/
	CellPubKey GetPubKey() const;

	/**
	* Create a DER-serialized signature.
	* The test_case parameter tweaks the deterministic nonce.
	*/
	bool Sign(const uint256& hash, std::vector<unsigned char>& vchSig, uint32_t test_case = 0) const;

	/**
	* Create a compact signature (65 bytes), which allows reconstructing the used public key.
	* The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
	* The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
	*                  0x1D = second key with even y, 0x1E = second key with odd y,
	*                  add 0x04 for compressed keys.
	*/
	bool SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const;

	//! Derive BIP32 child key.
	bool Derive(CellKey& keyChild, ChainCode &ccChild, unsigned int nChild, const ChainCode& cc) const;

	/**
	* Verify thoroughly whether a private key and a public key match.
	* This is done using a different mechanism than just regenerating it.
	*/
	bool VerifyPubKey(const CellPubKey& vchPubKey) const;

	//! Load private key and check that public key matches.
	//bool Load(CPrivKey& privkey, CellPubKey& vchPubKey, bool fSkipCheck);
};

const unsigned int BIP32_EXTKEY_SIZE = 74;

struct CellExtPubKey
{
	unsigned char nDepth;
	unsigned char vchFingerprint[4];
	unsigned int nChild;
	ChainCode chaincode;
	CellPubKey pubkey;

	friend bool operator==(const CellExtPubKey &a, const CellExtPubKey &b);

	void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
	void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
	bool Derive(CellExtPubKey& out, unsigned int nChild) const;

	//void Serialize(CellSizeComputer& s) const
	
	template <typename Stream>
	void Serialize(Stream& s) const;
	
	template <typename Stream>
	void Unserialize(Stream& s);
};

struct CellExtKey
{
	unsigned char nDepth;
	unsigned char vchFingerprint[4];
	unsigned int nChild;
	ChainCode chaincode;
	CellKey key;

	friend bool operator==(const CellExtKey& a, const CellExtKey& b);

	void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
	void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
	bool Derive(CellExtKey& out, unsigned int nChild) const;
	CellExtPubKey Neuter() const;
	void SetMaster(const unsigned char* seed, unsigned int nSeedLen);

	template <typename Stream>
	void Serialize(Stream& s) const;
	
	template <typename Stream>
	void Unserialize(Stream& s);
};
