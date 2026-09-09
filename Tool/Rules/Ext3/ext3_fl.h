#ifndef EXT3_FL_H
#define EXT3_FL_H

#include "../../rule.h"

/**
 * \brief Ext3 files evaluation rule
 * \author Janine Schneider
 * \author Josef Ilg
 * \author Marie Becker
 * \date May, 2026
 */
class Ext3_fl : public Rule
{
public:
    Ext3_fl();
    ~Ext3_fl();

    /**
     * \brief Rule evaluation of possible Ext3 block sequences representing files (active, inactive and unknown)
     * \param input Input AddressNodes whose byte source is searched for Ext3 file structures
     * \return AddressNodeList containing reconstructionNodes representing found files and their corresponding metadata
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    /**
     * \brief Parses indirect block pointer
     * \param node The AddressNode whose byte source is read to resolve block pointers
     * \param startAddress Block start address
     * \param blockSize File system block size
     * \param lowerBound Lower access bound within the node's byte source
     * \param upperBound Upper access bound within the node's byte source
     * \return Vector of blocks
     */
    std::vector<uint32_t> parseBlocks(AddressNodePtr node, uint32_t startAddress, uint32_t blockSize, uint64_t lowerBound, uint64_t upperBound);

    /**
     * \brief Converts file system blocks to start and end address in byte
     * \param blocks Blocks to be converted
     * \param blockSize File system block size
     * \param indexOffset Address offset relative to the upper most input node's byte source
     * \return Pair of start and end address
     */
    std::vector<std::pair<uint64_t, uint64_t>> convertBlocks(std::vector<uint64_t> blocks, uint32_t blockSize, uint64_t indexOffset);
};

#endif // EXT3_FL_H
