#include "ext4_fl.h"

#include <iostream>

Ext4_fl::Ext4_fl() {}

Ext4_fl::~Ext4_fl() {}

AddressNodeList Ext4_fl::evaluate(AddressNodeList input)
{
    AddressNodeList output;
    for (AddressNodePtr& node : input) {
        // Result vector
        std::vector<std::pair<uint64_t, uint64_t>> blockSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> blockSeqences;
        std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
        AddressNodeList new_children;
        std::string tag = "ext4_fl";

        try {
            // Helper
            uint32_t inodeCounter = 1;
            std::map<int, std::vector<std::string>> names;
            std::map<int, std::vector<std::pair<uint64_t, uint64_t>>> nameAndBlocks;

            // ContentData
            std::pair<uint64_t, uint64_t> contentData = node->m_data.front().front();

            // Superblock
            std::pair<uint64_t, uint64_t> superBlock = node->m_metadata.front().front();
            // Store block size in byte
            uint64_t blockSize = std::pow(2, 10 + get_32bit(node, superBlock.first, 0x18, superBlock.first, superBlock.second));

            /*
             * Store inode size in byte
             *
             * There are inode record size (Store in 0x58 of Superblock) and actual used size (128Byte + i_extra_isize).
             * Record size is filtered out here and used in the following.
             */
            uint16_t inodeSize = get_16bit(node, superBlock.first, 0x58, superBlock.first, superBlock.second);

            std::vector<std::vector<std::pair<uint64_t, uint64_t>>> inodeTables = node->m_metadata;
            inodeTables.erase(inodeTables.begin()); // Remove superblock from inode tables list

            // Go through all inode tables
            for (uint64_t i = 0; i < inodeTables.size(); i++) {
                std::pair<uint64_t, uint64_t> inodeTable = inodeTables.at(i).front();
                uint64_t tableSize = inodeTable.second - inodeTable.first + 1;

                // Iterate through all inodes (use inode record size)
                for (uint64_t position = 0; position < tableSize; position += inodeSize) {
                    uint16_t i_mode = get_16bit(node, inodeTable.first, position, inodeTable.first, inodeTable.second);
                    uint32_t size_lower32bit = get_32bit(node, inodeTable.first, position + 4, inodeTable.first, inodeTable.second);
                    uint32_t size_upper32bit = get_32bit(node, inodeTable.first, position + 0x6C, inodeTable.first, inodeTable.second);

                    uint64_t size = (((uint64_t)size_upper32bit) << 32) | size_lower32bit;

                    if ((i_mode == 0x0000 && size == 0x0000000000000000) || ((inodeCounter != 2) && (inodeCounter < 11))) {
                        inodeCounter++;
                        continue;
                    }

                    /*
                     * TODO Symbol links
                    */

                    // Get Inode flags
                    uint32_t i_flags = get_32bit(node, inodeTable.first, position + 0x20, inodeTable.first, inodeTable.second);

                    // Extent Tree is used?
                    bool EXT4_EXTENTS_FL = i_flags & 0x80000 ? true : false;
                    // Inline?
                    bool EXT4_INLINE_DATA_FL = i_flags & 0x10000000 ? true : false;
                    //Directory has hashed indexes
                        //// bool EXT4_INDEX_FL = i_flags & 0x1000 ? true : false;

                    if (EXT4_INLINE_DATA_FL || size <= 60) {
                        // TODO Inline data
                        continue;
                    }
                    else if (EXT4_EXTENTS_FL) {
                        // Blocks are stored in a tree structure
                        blockSeqence = readTree(node, inodeTable.first + position + 0x28, &contentData, &blockSize);
                    }
                    else {
                        // It's the traditional addressing how it was used in ext2/3
                        blockSeqence = direct_indirectBlockAdressing(node, &position, &inodeTable, &blockSize, &contentData);
                    }

                    if (blockSeqence.empty()) {
                        inodeCounter++;
                        continue;
                    }

                    // store inode entry in metadata sequence
                    metadataSeqence.push_back(std::make_pair(inodeTable.first + position, (inodeTable.first + position + inodeSize) - 1));

                    /*
                    * trim block sequence to end at actual end of file
                    */
                    // 
                    uint64_t remainingFileSize = size;
                    std::vector<std::pair<uint64_t, uint64_t>> trimmedBlockSeqence;
                    for (const auto& range : blockSeqence) {
                        if (remainingFileSize == 0) {
                            // reached end of file data
                            break;
                        }
                        uint64_t rangeLength = range.second - range.first + 1;

                        if (rangeLength <= remainingFileSize) {
                            // eintire block range is part of  file data
                            trimmedBlockSeqence.push_back(range);
                            remainingFileSize -= rangeLength;
                        }
                        else {
                            trimmedBlockSeqence.push_back({ range.first, range.first + remainingFileSize - 1 });
                            remainingFileSize = 0;
                        }
                    }
                    blockSeqence = std::move(trimmedBlockSeqence);


                    /*
                     * Get names
                     *
                     * INFO: - Names are stored in the blocks of a directory.
                     *       - Currently, names are not needed. However, they could be used when tags are integrated in future
                    */

                    /*
                    // Check if inode references a directory
                    bool is_directory = i_mode & 0x4000 ? true : false;

                    if (is_directory)
                    {
                        if (EXT4_INDEX_FL)
                        { // Hash Tree Directories

                            // TODO
                        }
                        else
                        { // Linear (Classic) Directories

                            for (std::pair<uint64_t, uint64_t> block : blockSeqence)
                            {
                                uint16_t rec_len = 0;
                                for (uint64_t offset = block.first; offset < block.second; offset += (uint64_t)rec_len)
                                {
                                    uint32_t dir_inode = get_32bit(node, offset, 0x0, contentData.first, contentData.second);
                                    rec_len = get_16bit(node, offset, 0x4, contentData.first, contentData.second);
                                    if (dir_inode == 0)
                                        continue; // Unused directory entries are signified by inode = 0
                                    uint8_t name_len = get_8bit(node, offset, 0x6, contentData.first, contentData.second);

                                    // Get name
                                    for (uint16_t i = 0; i < name_len; i++)
                                    {
                                        bytes[i] = node->root->data->at(offset, 0x8 + i, contentData.first, contentData.second);
                                    }
                                    std::string name(reinterpret_cast<char *>(bytes));
                                    if (name.size() > name_len)
                                        name.erase(name_len);

                                    // Insert name into map
                                    if (names.find(dir_inode) != names.end())
                                    { // Inode already exists in map
                                        names.find(dir_inode)->second.push_back(name);
                                    }
                                    else
                                    { // Inode does not exist in map
                                        names.insert(std::make_pair<int, std::vector<std::string>>(dir_inode, {name}));
                                    }
                                }
                            }
                        }
                    }

                    nameAndBlocks.insert(std::make_pair(inodeCounter, blockSeqence));
                    */



                    blockSeqences.push_back(blockSeqence);
                    blockSeqence.clear();
                    metadataSeqences.push_back(metadataSeqence);
                    metadataSeqence.clear();


                    // relative partition-offset to absolut ByteContainer-offset
                    for (auto& seq : blockSeqences) {
                        for (auto& pair : seq) {
                            pair.first += contentData.first;
                            pair.second += contentData.first;
                        }
                    }

                    if (!blockSeqences.empty() && !blockSeqences.front().empty()) {

                        AddressNodePtr new_node = std::make_shared<reconstructionNode>(blockSeqences, metadataSeqences, tag);
                        if (new_node != nullptr) {
                            new_children.push_back(new_node);
                        }
                    }

                    blockSeqences.clear();
                    metadataSeqences.clear();
                    inodeCounter++;
                }


            }

            node->add_children(new_children);

            /*
            * Combining names with sequence blocks
            * Currently not needed, however, it is relevant for a potential later usage with tags
            */

            /*
            std::string name;
            std::vector<std::pair<std::string, std::pair<uint64_t, uint64_t>>>
                result;
            std::vector<std::vector<std::pair<std::string, std::pair<uint64_t, uint64_t>>>> results;
            for (std::pair<uint64_t, std::vector<std::pair<uint64_t, uint64_t>>> nameToBlocks : nameAndBlocks)
            {
                name = "";
                if (names.find(nameToBlocks.first) != names.end())
                    name = names.find(nameToBlocks.first)->second.front();

                for (std::pair<uint64_t, uint64_t> sequence : nameToBlocks.second)
                {
                    result.push_back(std::make_pair(name, sequence));
                }
                results.push_back(result);
                result.clear();
            }
            */
        }
        catch (std::exception& e)

        {
            std::cout << e.what() << std::endl;
        }

        output.insert(output.end(), new_children.begin(), new_children.end());
        new_children.clear();
    }

    return output;
}

std::vector<std::pair<uint64_t, uint64_t>> Ext4_fl::readTree(AddressNodePtr node, uint64_t startStruct, std::pair<uint64_t, uint64_t>* contentData, uint64_t* blockSize)
{
    uint16_t eh_magic = get_16bit(node, 0, startStruct, contentData->first, contentData->second);
    if (eh_magic != 0xF30A)
        throw "Unknown state";

    uint16_t eh_depth = get_16bit(node, 0, startStruct + 0x6, contentData->first, contentData->second);
    uint16_t eh_entries = get_16bit(node, 0, startStruct + 0x2, contentData->first, contentData->second);

    if (eh_depth > 0) {
        std::vector<std::pair<uint64_t, uint64_t>> blockSeqence;

        for (u_int64_t e = 0; e < eh_entries; e++) {
            uint64_t start_extent_idx = startStruct + 0xc * (1 + e);

            unsigned char bytes[8];
            uint64_t ei_leaf;
            bytes[0] = node->root->data->at(0, start_extent_idx + 0x4, contentData->first, contentData->second);
            bytes[1] = node->root->data->at(0, start_extent_idx + 0x5, contentData->first, contentData->second);
            bytes[2] = node->root->data->at(0, start_extent_idx + 0x6, contentData->first, contentData->second);
            bytes[3] = node->root->data->at(0, start_extent_idx + 0x7, contentData->first, contentData->second);
            bytes[4] = node->root->data->at(0, start_extent_idx + 0x8, contentData->first, contentData->second);
            bytes[5] = node->root->data->at(0, start_extent_idx + 0x9, contentData->first, contentData->second);
            bytes[6] = 0;
            bytes[7] = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&ei_leaf));

            std::vector<std::pair<uint64_t, uint64_t>> blocks = readTree(node, *blockSize * ei_leaf, contentData, blockSize);
            blockSeqence.insert(blockSeqence.end(), blocks.begin(), blocks.end());
        }
        return blockSeqence;
    }
    else {
        std::vector<std::pair<uint64_t, uint64_t>> blockSeqence;
        for (u_int64_t e = 0; e < eh_entries; e++) {
            uint64_t start_extent = startStruct + 0xc * (1 + e);
            uint16_t ee_len = get_16bit(node, 0, start_extent + 0x4, contentData->first, contentData->second); // <- Fix
            if (ee_len > 32768)
                ee_len -= 32768;

            unsigned char bytes[8];
            uint64_t ee_start;
            bytes[0] = node->getByteSource()->at(0, start_extent + 0x8, contentData->first, contentData->second);
            bytes[1] = node->getByteSource()->at(0, start_extent + 0x9, contentData->first, contentData->second);
            bytes[2] = node->getByteSource()->at(0, start_extent + 0xa, contentData->first, contentData->second);
            bytes[3] = node->getByteSource()->at(0, start_extent + 0xb, contentData->first, contentData->second);
            bytes[4] = node->getByteSource()->at(0, start_extent + 0x6, contentData->first, contentData->second);
            bytes[5] = node->getByteSource()->at(0, start_extent + 0x7, contentData->first, contentData->second);
            bytes[6] = 0;
            bytes[7] = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&ee_start));

            blockSeqence.push_back(std::make_pair(ee_start * *blockSize, ((ee_start + ee_len) * *blockSize) - 1));
        }
        return blockSeqence;
    }
}

std::vector<std::pair<uint64_t, uint64_t>> Ext4_fl::direct_indirectBlockAdressing(AddressNodePtr node, uint64_t* position,
    std::pair<uint64_t, uint64_t>* inodeTable,
    uint64_t* blockSize,
    std::pair<uint64_t, uint64_t>* contentData)
{
    uint32_t num32 = 0;
    std::vector<uint64_t> totalBlocks;
    std::vector<uint32_t> directBlocks;
    std::vector<uint32_t> indirectBlocks;
    std::vector<std::pair<uint64_t, uint64_t>> blockSeqence;

    // Store direct block pointers
    // There are twelfe  -> 12 * 4 = 48
    for (uint32_t i = 0; i < 48; i++) {
        num32 = get_32bit(node, inodeTable->first, *position + 40 + i, inodeTable->first, inodeTable->second);
        if (num32 != 0) {
            directBlocks.push_back(num32);
        }

        i += 3;
    }

    // totalBlocks = directBlocks;//@DM18

    // Store single indirect block pointer
    num32 = get_32bit(node, inodeTable->first, *position + 88, inodeTable->first, inodeTable->second);

    if (num32 != 0) {
        std::vector<uint32_t> temp;
        indirectBlocks.push_back(num32);
        temp = parseBlocks(node, num32, *blockSize, contentData);          //@DM18
        directBlocks.insert(directBlocks.end(), temp.begin(), temp.end()); //@DM18 store direct blocks
    }


    num32 = get_32bit(node, inodeTable->first, *position + 92, inodeTable->first, inodeTable->second);

    if (num32 != 0) {
        std::vector<uint32_t> temp;
        std::vector<uint32_t> temp2;
        //@DM18 begin
        // store double indirect block
        indirectBlocks.push_back(num32);
        temp = parseBlocks(node, num32, *blockSize, contentData); //@DM18
        // store single indirect block
        indirectBlocks.insert(indirectBlocks.end(), temp.begin(), temp.end());
        // get direct blocks by parsing single indirect blocks
        for (uint32_t i = 0; i < temp.size(); i++) {
            temp2 = parseBlocks(node, temp.at(i), *blockSize, contentData);
            directBlocks.insert(directBlocks.end(), temp2.begin(), temp2.end());
        }
        //@DM18 end
    }

    // Store triple indirect block pointer
    num32 = get_32bit(node, inodeTable->first, *position + 96, inodeTable->first, inodeTable->second);

    if (num32 != 0) {
        //@DM18 begin
        std::vector<uint32_t> temp;
        std::vector<uint32_t> temp2;
        std::vector<uint32_t> temp3;

        // get and store double indirect block pointer
        temp = parseBlocks(node, num32, *blockSize, contentData); //@DM18
        indirectBlocks.insert(indirectBlocks.end(), temp.begin(), temp.end());
        // get and store single indirect block pointer
        for (uint32_t i = 0; i < temp.size(); i++) {
            temp2 = parseBlocks(node, temp.at(i), *blockSize, contentData); //@DM18
            indirectBlocks.insert(indirectBlocks.end(), temp2.begin(), temp2.end());
            // get and store direct block pointer
            for (uint32_t j = 0; j < temp2.size(); j++) {
                temp3 = parseBlocks(node, temp2.at(i), *blockSize, contentData); //@DM18
                directBlocks.insert(directBlocks.end(), temp2.begin(), temp2.end());
            }
        }
        //@DM18 end
    }
    totalBlocks.insert(totalBlocks.end(), directBlocks.begin(), directBlocks.end()); //@DM18

    if (!totalBlocks.empty()) {
        std::sort(totalBlocks.begin(), totalBlocks.end());
        blockSeqence = convertBlocks(node, totalBlocks, *blockSize, contentData);
    }
    else {
        blockSeqence.push_back(std::make_pair(0, 0));
    }

    return blockSeqence;
}

std::vector<uint32_t> Ext4_fl::parseBlocks(AddressNodePtr node, uint32_t startAddress, uint32_t blockSize, std::pair<uint64_t, uint64_t>* contentData)
{
    uint32_t num = 0;
    std::vector<uint32_t> blocks;
    uint64_t position = startAddress * blockSize;

    for (uint32_t i = 0; i < 4096; i++) {
        num = get_32bit(node, position, i, contentData->first, contentData->second);
        if (num == 0) {
            break;
        }
        blocks.push_back(num);
        i += 3;
    }

    return blocks;
}

std::vector<std::pair<uint64_t, uint64_t>> Ext4_fl::convertBlocks(AddressNodePtr node, std::vector<uint64_t> blocks, uint32_t blockSize, std::pair<uint64_t, uint64_t>* contentData)
{
    /*
     * Handle possible fragmentation
     */
    std::vector<std::vector<uint64_t>> groupgedBlocks;
    std::vector<uint64_t> helper;
    helper.push_back(blocks.front());

    for (size_t i = 0; i < blocks.size(); i++) {
        // Reached end
        if ((i + 1) == blocks.size()) {
            groupgedBlocks.push_back(helper);
            break;
        }

        // Not fragmented
        if (blocks.at(i) + 1 == blocks.at(i + 1)) {
            helper.push_back(blocks.at(i + 1));
        }
        // Fragmented
        else {
            // Push all data until this point to groupgedBlocks and start by zero again
            groupgedBlocks.push_back(helper);
            helper.clear();
            helper.push_back(blocks.at(i + 1));
        }
    }

    /*
     * Store Start and end address of the grouped groups
     */
    std::pair<uint64_t, uint64_t> res_pair;
    std::vector<std::pair<uint64_t, uint64_t>> res_vec;

    for (auto it = groupgedBlocks.begin(); it != groupgedBlocks.end(); ++it) {
        // Calculate start address
        res_pair.first = contentData->first + it->front() * blockSize;
        // Calculate end address
        res_pair.second = (res_pair.first + (it->size() * blockSize)) - 1;

        res_vec.push_back(res_pair);
    }

    return res_vec;
}

uint32_t Ext4_fl::get_32bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    unsigned char bytes[4];
    uint32_t num32 = 0;
    bytes[0] = node->root->data->at(offset, position, lowerBound, upperBound);
    bytes[1] = node->root->data->at(offset, position + 1, lowerBound, upperBound);
    bytes[2] = node->root->data->at(offset, position + 2, lowerBound, upperBound);
    bytes[3] = node->root->data->at(offset, position + 3, lowerBound, upperBound);
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));
    return num32;
}

uint16_t Ext4_fl::get_16bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    unsigned char bytes[2];
    uint16_t num16 = 0;
    bytes[0] = node->root->data->at(offset, position, lowerBound, upperBound);
    bytes[1] = node->root->data->at(offset, position + 1, lowerBound, upperBound);
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num16));
    return num16;
}

uint8_t Ext4_fl::get_8bit(AddressNodePtr node, uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    unsigned char bytes[1];
    uint8_t num8 = 0;
    bytes[0] = node->root->data->at(offset, position, lowerBound, upperBound);
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint8_t), reinterpret_cast<unsigned char*>(&num8));
    return num8;
}