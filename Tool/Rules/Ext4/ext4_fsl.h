#ifndef EXT4_FSL_H
#define EXT4_FSL_H

#include "../../rule.h"
#include <cmath>
#include <algorithm>
#include <iostream>

/**
 * \brief Ext4 file system evaluation rule
 */
class Ext4_fsl : public Rule
{
public:
    Ext4_fsl();
    ~Ext4_fsl();

    /**
     * \brief Rule evaluation of possible block sequences representing an Ext4 file system
     * \param input Input AddressNodes whose byte source is searched for an Ext4 file system structure
     * \return AddressNodeList containing reconstructionNodes representing the found file system (data) and its corresponding metadata (superblock, inode_table_1, inode_table_2, ...)
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    ByteContainer* m_rawData;
    uint32_t get_32bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound);
    uint32_t get_16bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound);
    uint8_t get_8bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound);
};

#endif // EXT4_FSL_H
