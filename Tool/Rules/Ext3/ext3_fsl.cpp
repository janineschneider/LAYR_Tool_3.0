#include "ext3_fsl.h"

#include <algorithm>
#include <iostream>

Ext3_fsl::Ext3_fsl() {}

Ext3_fsl::~Ext3_fsl() {}

AddressNodeList Ext3_fsl::evaluate(AddressNodeList input)
{
    AddressNodeList output;
    for (AddressNodePtr node : input) {
        // Result vector
        ByteContainer* source = node->getByteSource();
        std::vector<std::pair<uint64_t, uint64_t>> contentSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> contentSeqences;
        std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
        std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence2;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
        std::string tag = "ext3_fsl";

        // Helper
        // LowerBound also functions as index offset to lowest input level
        // TODO Use more than first object
        // TODO Handle fragmentation
        uint64_t lowerBound = node->m_data.front().front().first;
        uint64_t upperBound = node->m_data.back().back().second;
        uint64_t indexOffset = node->m_data.front().front().first;
        std::pair<uint64_t, uint64_t> indexPair;

        try {

            // Store boot sector indices as file system metadata
            uint16_t bootSectorStart = indexOffset + 1024;
            uint16_t bootSectorEnd = indexOffset + 2047;
            indexPair = std::make_pair(bootSectorStart, bootSectorEnd);
            metadataSeqence.push_back(indexPair);
            metadataSeqences.push_back(metadataSeqence);
            metadataSeqence.clear();

            // Variable initialization
            unsigned char bytes[4];
            uint16_t num16 = 0;
            uint32_t num32 = 0;

            // Validate Ext Magic Number (Offset 56 / 0x38 in Superblock)
            bytes[0] = source->at(indexOffset + 1024, 56, lowerBound, upperBound);
            bytes[1] = source->at(indexOffset + 1024, 57, lowerBound, upperBound);
            num16 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num16));

            // Check Ext4 Incompatible Features (Offset 96 / 0x60 in Superblock)
            bytes[0] = source->at(indexOffset + 1024, 96, lowerBound, upperBound);
            bytes[1] = source->at(indexOffset + 1024, 97, lowerBound, upperBound);
            bytes[2] = source->at(indexOffset + 1024, 98, lowerBound, upperBound);
            bytes[3] = source->at(indexOffset + 1024, 99, lowerBound, upperBound);
            num32 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));

            // Extents or 64-bit feature flags
            bool is_ext4 = (num32 & 0x40) || (num32 & 0x80);

            // Fail Ext3 if magic number is invalid OR if Ext4 features are present
            if (num16 != 0xEF53 || is_ext4) {
                throw std::runtime_error("EXT3_fsl validation failed: Not an Ext3 filesystem");
            }

            // Essential Ext3 file system information
            uint64_t blockSize = 0;
            uint64_t blockNumber = 0;
            uint64_t fileSystemSize = 0;
            uint32_t reservedBlocks = 0;
            uint32_t inodesPerGroup = 0;
            uint16_t inodeSize = 0;

            // Store block size in byte
            bytes[0] = source->at(indexOffset + 1024, 24, lowerBound, upperBound);
            bytes[1] = source->at(indexOffset + 1024, 25, lowerBound, upperBound);
            bytes[2] = source->at(indexOffset + 1024, 26, lowerBound, upperBound);
            bytes[3] = source->at(indexOffset + 1024, 27, lowerBound, upperBound);
            num32 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));
            blockSize = 1024 << num32;

            // Store numbers of blocks in file system
            bytes[0] = source->at(indexOffset + 1024, 4, lowerBound, upperBound);
            bytes[1] = source->at(indexOffset + 1024, 5, lowerBound, upperBound);
            bytes[2] = source->at(indexOffset + 1024, 6, lowerBound, upperBound);
            bytes[3] = source->at(indexOffset + 1024, 7, lowerBound, upperBound);
            num32 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));
            blockNumber = num32;
            fileSystemSize = blockNumber * blockSize;

            // Block where first block group starts
            bytes[0] = source->at(indexOffset + 1024, 20, lowerBound, upperBound);
            bytes[1] = source->at(indexOffset + 1024, 21, lowerBound, upperBound);
            bytes[2] = source->at(indexOffset + 1024, 22, lowerBound, upperBound);
            bytes[3] = source->at(indexOffset + 1024, 23, lowerBound, upperBound);
            num32 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));
            reservedBlocks = num32;

            // Store number of inodes per group
            bytes[0] = source->at(indexOffset + 1024, 40, lowerBound, upperBound);
            bytes[1] = source->at(indexOffset + 1024, 41, lowerBound, upperBound);
            bytes[2] = source->at(indexOffset + 1024, 42, lowerBound, upperBound);
            bytes[3] = source->at(indexOffset + 1024, 43, lowerBound, upperBound);
            num32 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));
            inodesPerGroup = num32;

            // Store inode size in byte
            bytes[0] = source->at(indexOffset + 1024, 88, lowerBound, upperBound);
            bytes[1] = source->at(indexOffset + 1024, 89, lowerBound, upperBound);
            num16 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num16));
            inodeSize = num16;

            // Store file system start and end as indices
            indexPair = std::make_pair(indexOffset, (indexOffset + fileSystemSize) - 1);
            contentSeqence.push_back(indexPair);
            contentSeqences.push_back(contentSeqence);
            contentSeqence.clear();

            // Extract group descriptor tables for further analysis
            uint64_t groupDescriptorTablesStart = indexOffset + (blockSize * (reservedBlocks + 1));
            uint64_t groupDescriptorTablesEnd = indexOffset + ((blockSize * (reservedBlocks + 2)) - 1);
            std::vector<unsigned char> groupDescriptorTables(source->copy2Vector(groupDescriptorTablesStart, groupDescriptorTablesEnd));

            // Store block group information
            std::vector<unsigned char> groupDescriptorTable;
            uint32_t inodes = inodesPerGroup * inodeSize;
            inodes = inodes / blockSize;

            // From here on groupDescriptorTablesEnd and groupDescriptorTablesStart act as an index, which labels start and end of one entry
            groupDescriptorTablesEnd = groupDescriptorTablesStart + 31;

            while (true) {
                // Use group descriptor tables to get inode tables
                groupDescriptorTable.clear();
                std::copy(groupDescriptorTables.begin(), groupDescriptorTables.begin() + 32, std::back_inserter(groupDescriptorTable));
                groupDescriptorTables.erase(groupDescriptorTables.begin(), groupDescriptorTables.begin() + 32);

                // Check if end is reached
                bool test = std::all_of(groupDescriptorTable.begin(),
                    groupDescriptorTable.end(), [](int i)
                    { return i == 0; });
                if (test) {
                    break;
                }

                // Store group descriptor table indices as file system metadata
                indexPair = std::make_pair(groupDescriptorTablesStart, groupDescriptorTablesEnd);
                metadataSeqence.push_back(indexPair);
                // Next entry
                groupDescriptorTablesStart = groupDescriptorTablesEnd + 1;
                groupDescriptorTablesEnd += 32;

                // Store inode table block address
                bytes[0] = groupDescriptorTable.at(8);
                bytes[1] = groupDescriptorTable.at(9);
                bytes[2] = groupDescriptorTable.at(10);
                bytes[3] = groupDescriptorTable.at(11);
                num32 = 0;
                std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));

                // Store inode table indices as file system metadata
                uint64_t InodeTableStart = indexOffset + (num32 * blockSize);
                uint64_t InodeTableEnd = indexOffset + (((inodes + num32 - 1) * blockSize) - 1);
                indexPair = std::make_pair(InodeTableStart, InodeTableEnd);
                metadataSeqence2.push_back(indexPair);
            }

            metadataSeqences.push_back(metadataSeqence);
            metadataSeqences.push_back(metadataSeqence2);
            metadataSeqence.clear();
            metadataSeqence2.clear();

            AddressNodePtr new_child = std::make_shared<reconstructionNode>(contentSeqences, metadataSeqences, tag);
            node->add_children(std::vector{ new_child });
            output.push_back(new_child);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }

    if (output.empty()) {
        throw std::runtime_error("Ext3_fsl: no valid Ext3 filesystem found in any input node");
    }

    return output;
}