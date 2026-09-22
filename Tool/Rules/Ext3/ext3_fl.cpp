#include "ext3_fl.h"

#include <iostream>
#include <algorithm>

Ext3_fl::Ext3_fl() {}

Ext3_fl::~Ext3_fl() {}

AddressNodeList Ext3_fl::evaluate(AddressNodeList input)
{
    AddressNodeList output;
    for (AddressNodePtr& node : input) {
        // Result vector
        std::vector<std::pair<uint64_t, uint64_t>> blockSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> blockSeqences;
        std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
        AddressNodeList new_children;
        std::string tag = "ext3_fl";


        // Helper
        // LowerBound also functions as index offset to lowest input level
        // TODO Use more than first object
        // TODO Handle fragmentation
        uint64_t lowerBound = node->m_data.front().front().first;
        uint64_t upperBound = node->m_data.back().back().second;
        uint64_t indexOffset = node->m_data.front().front().first;

        uint64_t lowerBoundMetadata = node->m_metadata.front().front().first;
        uint64_t upperBoundMetadata = node->m_metadata.front().front().second;
        uint64_t indexOffsetMetadata = node->m_metadata.front().front().first;

        try {
            // Helper
            uint32_t inodeCounter = 1;
            uint64_t position = 0;

            // Variable initialization
            unsigned char bytes[4];
            uint64_t num = 0; //@DM14

            // Essential Ext3 inode/file information
            std::vector<uint64_t> totalBlocks;
            std::vector<uint32_t> directBlocks;
            std::vector<uint32_t> indirectBlocks;

            // Store inode size in byte
            bytes[0] = node->root->data->at(indexOffsetMetadata, 88, lowerBoundMetadata, upperBoundMetadata);
            bytes[1] = node->root->data->at(indexOffsetMetadata, 89, lowerBoundMetadata, upperBoundMetadata);
            num = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
            uint16_t inodeSize = num;

            // Store block size in byte
            bytes[0] = node->root->data->at(indexOffsetMetadata, 24, lowerBoundMetadata, upperBoundMetadata);
            bytes[1] = node->root->data->at(indexOffsetMetadata, 25, lowerBoundMetadata, upperBoundMetadata);
            bytes[2] = node->root->data->at(indexOffsetMetadata, 26, lowerBoundMetadata, upperBoundMetadata);
            bytes[3] = node->root->data->at(indexOffsetMetadata, 27, lowerBoundMetadata, upperBoundMetadata);
            num = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
            uint32_t blockSize = 1024 << num;

            for (auto it = node->m_metadata.back().begin(); it != node->m_metadata.back().end(); ++it) {
                std::vector<unsigned char> inodeTable = node->root->data->copy2Vector(it->first, it->second);
                while (position < inodeTable.size()) {
                    // Break if inode essentials (file mode, size) are empty
                    if (inodeSize == 0) {
                        std::cerr << "Error: Calculated inodeSize is 0. Aborting node evaluation.\n";
                        break;
                    }
                    if (((inodeTable.at(position) == 0x00) &&
                        (inodeTable.at(position + 1) == 0x00) &&
                        (inodeTable.at(position + 4) == 0x00) &&
                        (inodeTable.at(position + 5) == 0x00) &&
                        (inodeTable.at(position + 6) == 0x00) &&
                        (inodeTable.at(position + 7) == 0x00)) ||
                        ((inodeCounter != 2) && (inodeCounter < 11))) {
                        inodeCounter++;
                        position += inodeSize;
                    }
                    else {
                        // Store Inode entry in metadata sequence
                        metadataSeqence.push_back(std::make_pair(it->first + position, (it->first + position + inodeSize) - 1));

                        // Store direct block pointers
                        // There are twelfe  -> 12 * 4 = 48
                        for (uint32_t i = 0; i < 48; i++) {
                            bytes[0] = inodeTable.at(position + 40 + i);
                            bytes[1] = inodeTable.at(position + 41 + i);
                            bytes[2] = inodeTable.at(position + 42 + i);
                            bytes[3] = inodeTable.at(position + 43 + i);
                            num = 0;
                            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
                            if (num != 0) {
                                directBlocks.push_back(num);
                            }

                            i += 3;
                        }

                        // totalBlocks = directBlocks;//@DM18

                        // Store single indirect block pointer
                        bytes[0] = inodeTable.at(position + 88);
                        bytes[1] = inodeTable.at(position + 89);
                        bytes[2] = inodeTable.at(position + 90);
                        bytes[3] = inodeTable.at(position + 91);
                        num = 0;
                        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));

                        if (num != 0) {
                            std::vector<uint32_t> temp;
                            indirectBlocks.push_back(num);
                            temp = parseBlocks(node, num, blockSize, lowerBound, upperBound);  //@DM18
                            directBlocks.insert(directBlocks.end(), temp.begin(), temp.end()); //@DM18 store direct blocks
                        }

                        // Store double indirect block pointer
                        bytes[0] = inodeTable.at(position + 92);
                        bytes[1] = inodeTable.at(position + 93);
                        bytes[2] = inodeTable.at(position + 94);
                        bytes[3] = inodeTable.at(position + 95);
                        num = 0;
                        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));

                        if (num != 0) {
                            std::vector<uint32_t> temp;
                            std::vector<uint32_t> temp2;
                            //@DM18 begin
                            // store double indirect block
                            indirectBlocks.push_back(num);
                            temp = parseBlocks(node, num, blockSize, lowerBound, upperBound); //@DM18
                            // store single indirect block
                            indirectBlocks.insert(indirectBlocks.end(), temp.begin(), temp.end());
                            // get direct blocks by parsing single indirect blocks
                            for (uint32_t i = 0; i < temp.size(); i++) {
                                temp2 = parseBlocks(node, temp.at(i), blockSize, lowerBound, upperBound);
                                directBlocks.insert(directBlocks.end(), temp2.begin(), temp2.end());
                            }
                            //@DM18 end
                        }

                        // Store triple indirect block pointer
                        bytes[0] = inodeTable.at(position + 96);
                        bytes[1] = inodeTable.at(position + 97);
                        bytes[2] = inodeTable.at(position + 98);
                        bytes[3] = inodeTable.at(position + 99);
                        num = 0;
                        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));

                        if (num != 0) {
                            //@DM18 begin
                            std::vector<uint32_t> temp;
                            std::vector<uint32_t> temp2;
                            std::vector<uint32_t> temp3;

                            // get and store double indirect block pointer
                            temp = parseBlocks(node, num, blockSize, lowerBound, upperBound); //@DM18
                            indirectBlocks.insert(indirectBlocks.end(), temp.begin(), temp.end());
                            // get and store single indirect block pointer
                            for (uint32_t i = 0; i < temp.size(); i++) {
                                temp2 = parseBlocks(node, temp.at(i), blockSize, lowerBound, upperBound); //@DM18
                                indirectBlocks.insert(indirectBlocks.end(), temp2.begin(), temp2.end());
                                // get and store direct block pointer
                                for (uint32_t j = 0; j < temp2.size(); j++) {
                                    temp3 = parseBlocks(node, temp2.at(i), blockSize, lowerBound, upperBound); //@DM18
                                    directBlocks.insert(directBlocks.end(), temp2.begin(), temp2.end());
                                }
                            }
                            //@DM18 end
                        }
                        totalBlocks.insert(totalBlocks.end(), directBlocks.begin(), directBlocks.end()); //@DM18

                        if (!totalBlocks.empty()) {
                            std::sort(totalBlocks.begin(), totalBlocks.end());
                            blockSeqence = convertBlocks(totalBlocks, blockSize, indexOffset);
                        }
                        else {
                            blockSeqence.push_back(std::make_pair(0, 0));
                        }

                        blockSeqences.push_back(blockSeqence);
                        blockSeqence.clear();
                        metadataSeqences.push_back(metadataSeqence);
                        metadataSeqence.clear();

                        if (!blockSeqences.empty() && !blockSeqences.front().empty()) {
                            AddressNodePtr new_node = std::make_shared<reconstructionNode>(blockSeqences, metadataSeqences, tag);
                            if (new_node != nullptr) {
                                new_children.push_back(new_node);
                            }
                        }

                        blockSeqences.clear();
                        metadataSeqences.clear();

                        inodeCounter++;
                        position += inodeSize;
                        directBlocks.clear();
                        indirectBlocks.clear();
                        totalBlocks.clear();
                    }
                }
                position = 0;
            }

            node->add_children(new_children);
            // output.contentdata = blockSeqences;
            // output.metadata = metadataSeqences;
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }

        output.insert(output.end(), new_children.begin(), new_children.end());
        new_children.clear();
    }
    return output;
}

std::vector<uint32_t> Ext3_fl::parseBlocks(AddressNodePtr node, uint32_t startAddress, uint32_t blockSize, uint64_t lowerBound, uint64_t upperBound) //@DM18
{
    //@DM18 begin
    unsigned char bytes[4];
    uint32_t num = 0;
    std::vector<uint32_t> blocks;
    uint64_t position = startAddress * blockSize;

    for (uint32_t i = 0; i < 4096; i++) { // read full block for addresses
        // Store block
        bytes[0] = node->root->data->at(position, i, lowerBound, upperBound);
        bytes[1] = node->root->data->at(position, i + 1, lowerBound, upperBound);
        bytes[2] = node->root->data->at(position, i + 2, lowerBound, upperBound);
        bytes[3] = node->root->data->at(position, i + 3, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
        if (num == 0) {
            break; // not full block used for addresses
        }
        blocks.push_back(num);
        i += 3;
    }
    //@DM18 end

    return blocks;
}

std::vector<std::pair<uint64_t, uint64_t>> Ext3_fl::convertBlocks(std::vector<uint64_t> blocks, uint32_t blockSize, uint64_t indexOffset)
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
        res_pair.first = indexOffset + it->front() * blockSize;
        // Calculate end address        
        res_pair.second = (res_pair.first + (it->size() * blockSize)) - 1;

        res_vec.push_back(res_pair);
    }

    return res_vec;
}
