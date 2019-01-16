#ifndef DATABASE_H
#define DATABASE_H

class DatabaseBlock
{
public:
    uint256 hashBlock;
    uint256 hashPrevBlock;
    uint256 hashSkipBlock;
    int height;

public:
    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hashBlock);
        READWRITE(hashPrevBlock);
        READWRITE(hashSkipBlock);
        READWRITE(height);
    }
};

bool DBInitialize();
const uint256 GetMaxHeightBlock();
int WriteBlockToDatabase(const MCBlock& block);
int GetDatabaseBlock(DatabaseBlock* block, const uint256& hashBlock);
MCBlockLocator MonitorGetLocator(const MCBlockIndex *pindex);

#endif
