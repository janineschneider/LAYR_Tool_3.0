#ifndef EXT4_FL_H
#define EXT4_FL_H

#include "../../rule.h"
#include <cmath>
#include <algorithm>
#include <map>

/**
 * \brief Ext4 files evaluation rule
 */
class Ext4_fl : public Rule
{
public:
    Ext4_fl();
    ~Ext4_fl();

    /**
     * \brief Rule evaluation of possible Ext4 block sequences representing files (active, inactive and unknown)
     * \param input Input AddressNodes whose byte source is searched for Ext4 file structures
     * \return AddressNodeList containing reconstructionNodes representing found files and their corresponding metadata
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    std::vector<std::pair<uint64_t, uint64_t>> readTree(AddressNodePtr node, uint64_t startStruct, std::pair<uint64_t, uint64_t>* contentData, uint64_t* blockSize);
    std::vector<std::pair<uint64_t, uint64_t>> direct_indirectBlockAdressing(AddressNodePtr node, uint64_t* position,
        std::pair<uint64_t, uint64_t>* inodeTable,
        uint64_t* blockSize,
        std::pair<uint64_t, uint64_t>* contentData);
    std::vector<uint32_t> parseBlocks(AddressNodePtr node, uint32_t startAddress, uint32_t blockSize, std::pair<uint64_t, uint64_t>* contentData);
    std::vector<std::pair<uint64_t, uint64_t>> convertBlocks(AddressNodePtr node, std::vector<uint64_t> blocks, uint32_t blockSize, std::pair<uint64_t, uint64_t>* contentData);
    uint32_t get_32bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound);
    uint16_t get_16bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound);
    uint8_t get_8bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound);
};

#endif // EXT4_FL_H
