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
bool WriteBlockToDatabase(const MCBlock& block, const std::shared_ptr<DatabaseBlock> dbBlock, size_t sz);
std::shared_ptr<DatabaseBlock> GetDatabaseBlock(const uint256& hashBlock);
MCBlockLocator MonitorGetLocator(const MCBlockIndex* pindex);

#endif
