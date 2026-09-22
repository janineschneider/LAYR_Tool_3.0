#include "ext4_fsl.h"
#include <iostream>

Ext4_fsl::Ext4_fsl() {}

Ext4_fsl::~Ext4_fsl() {}

AddressNodeList Ext4_fsl::evaluate(AddressNodeList input)
{
    AddressNodeList output;
    if (input.size() != 1) {

    }
    for (AddressNodePtr node : input) {
        // Result vector
        std::vector<std::pair<uint64_t, uint64_t>> contentSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> contentSeqences;
        std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
        AddressNodeList new_children;
        std::string tag = "ext4_fsl";
        ByteContainer* source = node->getByteSource();

        try {
            // Store boot sector indices as file system metadata
            uint64_t superBlockStart = node->m_data.front().front().first + 1024;
            uint64_t superBlockEnd = superBlockStart + 1023;

            /*
            // Validate Ext Magic Number (Offset 56 / 0x38 in Superblock)
            uint16_t magicNumber = get_16bit(node, superBlockStart, 0x38, superBlockStart, superBlockEnd);
            if (magicNumber != 0xEF53) {
                std::cerr << "[Ext4_fsl Error] Node at offset " << node->m_data.front().front().first
                    << " does not contain a valid Ext4 superblock (Magic: 0x"
                    << std::hex << magicNumber << std::dec << " != 0xEF53)." << std::endl;
                continue;
            }
            */
            std::pair<uint64_t, uint64_t> indexPair = std::make_pair(superBlockStart, superBlockEnd);
            metadataSeqence.push_back(indexPair);
            metadataSeqences.push_back(metadataSeqence);
            metadataSeqence.clear();

            // Store block size in byte
            uint64_t blockSize = std::pow(2, 10 + get_32bit(node, superBlockStart, 0x18, superBlockStart, superBlockEnd));
            // Store numbers of blocks in file system
            uint32_t blockNumber = get_32bit(node, superBlockStart, 0x4, superBlockStart, superBlockEnd);
            uint64_t fileSystemSize = blockNumber * blockSize;

            // blocks per group
            uint32_t s_blocks_per_group = get_32bit(node, superBlockStart, 0x20, superBlockStart, superBlockEnd);
            // number of groups (The number of block groups is the size of the device divided by the size of a block group).
            uint64_t number_block_groups = (fileSystemSize / ((uint64_t)s_blocks_per_group * blockSize)) + 1;

            uint32_t s_feature_incompat = get_32bit(node, superBlockStart, 0x60, superBlockStart, superBlockEnd);
            bool meta_bg = s_feature_incompat & 0x00000010;
            bool incompat_64bit = s_feature_incompat & 0x00000080;
            ////bool flex_bg = s_feature_incompat & 0x00000200;

            uint16_t groupDescriptorSize;
            if (incompat_64bit) {
                // Size of a group descriptor if 64 flag is set
                groupDescriptorSize = get_16bit(node, superBlockStart, 0xFE, superBlockStart, superBlockEnd);
            }
            else {
                groupDescriptorSize = 32;
            }

            // Store file system start and end as indices
            indexPair = std::make_pair(node->m_data.front().front().first, (node->m_data.front().front().first + fileSystemSize) - 1);
            contentSeqence.push_back(indexPair);
            contentSeqences.push_back(contentSeqence);
            contentSeqence.clear();

            // Store number of inodes per group
            uint32_t inodesPerGroup = get_32bit(node, superBlockStart, 0x28, superBlockStart, superBlockEnd);
            // Store inode size in byte
            uint16_t inodeSize = get_16bit(node, superBlockStart, 0x58, superBlockStart, superBlockEnd);

            // TODO: check if needed
            //        uint64_t size_flex_group;
            //        if (flex_bg)
            //             size_flex_group = std::pow(2, get_8bit(superBlockStart, 0x174, superBlockStart, superBlockEnd));

            if (meta_bg) {
                // TODO:
                // 1. number_blocks_in_metablockgroup = blockSize / groupdescriptorSize
                // 2. Iterate over all metablockgroups and extract their group_descriptors in group 0 of the metablockgroup
                // Hint: Group descriptors are only of the own metablockgroup.
                throw "Currently, no Meta Block Groups are supported!";
            }

            // Extract group descriptor tables for further analysis
            uint64_t groupDescriptorTablesStart;
            if (blockSize == 1024)
                groupDescriptorTablesStart = superBlockStart + blockSize;
            else
                groupDescriptorTablesStart = node->m_data.front().front().first + (blockSize);
            uint64_t groupDescriptorTablesEnd = groupDescriptorTablesStart + number_block_groups * groupDescriptorSize - 1;
            std::vector<unsigned char> groupDescriptorTables(source->copy2Vector(groupDescriptorTablesStart, groupDescriptorTablesEnd));

            // Store block group information
            std::vector<unsigned char> groupDescriptorTable;
            uint32_t inodes = inodesPerGroup * inodeSize;
            inodes = inodes / blockSize;

            // Loop helpers
            unsigned char bytes[8];
            uint32_t num32 = 0;
            uint64_t num64 = 0;

            while (true) {
                // Use group descriptor tables to get inode tables
                groupDescriptorTable.clear();
                // Get current descriptor table from the descriptor tables vector
                std::copy(groupDescriptorTables.begin(), groupDescriptorTables.begin() + groupDescriptorSize, std::back_inserter(groupDescriptorTable));

                // Check if end is reached
                if (groupDescriptorTables.size() == 0)
                    break;

                // Cut off the current descriptor table from the descriptor tables vector
                if (groupDescriptorTables.size() == groupDescriptorSize)
                    groupDescriptorTables.clear();
                else
                    groupDescriptorTables.erase(groupDescriptorTables.begin(), groupDescriptorTables.begin() + groupDescriptorSize);

                if (incompat_64bit) {
                    // Store inode table block address
                    bytes[0] = groupDescriptorTable.at(0x8);
                    bytes[1] = groupDescriptorTable.at(0x9);
                    bytes[2] = groupDescriptorTable.at(0xa);
                    bytes[3] = groupDescriptorTable.at(0xb);
                    // last 32 bits of the 64 bit address is located in the upper part of the group descriptor
                    bytes[4] = groupDescriptorTable.at(0x28);
                    bytes[5] = groupDescriptorTable.at(0x29);
                    bytes[6] = groupDescriptorTable.at(0x2a);
                    bytes[7] = groupDescriptorTable.at(0x2b);
                    num64 = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&num64));

                    // Store inode table indices as file system metadata
                    uint64_t InodeTableStart = node->m_data.front().front().first + (num64 * blockSize);
                    uint64_t InodeTableEnd = node->m_data.front().front().first + ((inodes + num64) * blockSize - 1);
                    indexPair = std::make_pair(InodeTableStart, InodeTableEnd);
                    metadataSeqence.push_back(indexPair);
                }
                else {
                    // Store inode table block address
                    bytes[0] = groupDescriptorTable.at(8);
                    bytes[1] = groupDescriptorTable.at(9);
                    bytes[2] = groupDescriptorTable.at(10);
                    bytes[3] = groupDescriptorTable.at(11);
                    num32 = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));

                    // Store inode table indices as file system metadata
                    uint64_t InodeTableStart = node->m_data.front().front().first + (num32 * blockSize);
                    uint64_t InodeTableEnd = node->m_data.front().front().first + ((inodes + num32) * blockSize - 1);
                    indexPair = std::make_pair(InodeTableStart, InodeTableEnd);
                }
                metadataSeqence.push_back(indexPair);
                metadataSeqences.push_back(metadataSeqence);
                metadataSeqence.clear();
            }

            AddressNodePtr new_node = std::make_shared<reconstructionNode>(contentSeqences, metadataSeqences, tag);
            metadataSeqences.clear();
            contentSeqences.clear();
            new_children.push_back(new_node);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        node->add_children(new_children);

        output.insert(output.end(), new_children.begin(), new_children.end());
        new_children.clear();
    }

    return output;
}

uint32_t Ext4_fsl::get_32bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    unsigned char bytes[4];
    uint32_t num32 = 0;
    ByteContainer* source = node->getByteSource();
    bytes[0] = source->at(offset, position, lowerBound, upperBound);
    bytes[1] = source->at(offset, position + 1, lowerBound, upperBound);
    bytes[2] = source->at(offset, position + 2, lowerBound, upperBound);
    bytes[3] = source->at(offset, position + 3, lowerBound, upperBound);
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));
    return num32;
}

uint32_t Ext4_fsl::get_16bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    unsigned char bytes[4];
    uint16_t num16 = 0;
    ByteContainer* source = node->getByteSource();
    bytes[0] = source->at(offset, position, lowerBound, upperBound);
    bytes[1] = source->at(offset, position + 1, lowerBound, upperBound);
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num16));
    return num16;
}

uint8_t Ext4_fsl::get_8bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    unsigned char bytes[4];
    uint8_t num8 = 0;
    ByteContainer* source = node->getByteSource();
    bytes[0] = source->at(offset, position, lowerBound, upperBound);
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint8_t), reinterpret_cast<unsigned char*>(&num8));
    return num8;
}