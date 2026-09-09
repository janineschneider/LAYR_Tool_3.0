#include "dos.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

DOS::DOS() {}

DOS::~DOS() {}

AddressNodeList DOS::evaluate(AddressNodeList input)
{
    AddressNodeList result;
    for (AddressNodePtr& node : input) {
        // Result vector
        std::vector<std::pair<uint64_t, uint64_t>> blockSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> blockSeqences;
        std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
        AddressNodeList new_children;
        std::string tag = "DOS";
        // Helper
        // LowerBound also functions as index offset to lowest input level
        // TODO Use more than first object
        // TODO Handle fragmentation
        uint64_t lowerBound = node->m_data.front().front().first;
        uint64_t upperBound = node->m_data.back().back().second;
        uint64_t indexOffset = node->m_data.front().front().first;
        std::pair<uint64_t, uint64_t> indexPair;

        try {
            // Variable initialization
            unsigned char bytes[4];
            uint32_t num32 = 0;

            // Seperate partition table entries
            std::vector<std::pair<uint32_t, uint32_t>> tableEntries;
            tableEntries.push_back(std::make_pair(indexOffset + 446, indexOffset + 461));
            tableEntries.push_back(std::make_pair(indexOffset + 462, indexOffset + 477));
            tableEntries.push_back(std::make_pair(indexOffset + 478, indexOffset + 493));
            tableEntries.push_back(std::make_pair(indexOffset + 494, indexOffset + 509));

            for (auto it = tableEntries.begin(); it != tableEntries.end(); ++it) {
                if ((node->root->data->at(it->first, 12, lowerBound, upperBound) == 0x00) &&
                    (node->root->data->at(it->first, 13, lowerBound, upperBound) == 0x00) &&
                    (node->root->data->at(it->first, 14, lowerBound, upperBound) == 0x00) &&
                    (node->root->data->at(it->first, 15, lowerBound, upperBound) == 0x00)) {
                    break;
                }
                else {
                    // Store starting LBA address in sectors
                    bytes[0] = node->root->data->at(it->first, 8, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(it->first, 9, lowerBound, upperBound);
                    bytes[2] = node->root->data->at(it->first, 10, lowerBound, upperBound);
                    bytes[3] = node->root->data->at(it->first, 11, lowerBound, upperBound);
                    num32 = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));
                    indexPair.first = indexOffset + (num32 * 512);

                    // Store size in sectors
                    bytes[0] = node->root->data->at(it->first, 12, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(it->first, 13, lowerBound, upperBound);
                    bytes[2] = node->root->data->at(it->first, 14, lowerBound, upperBound);
                    bytes[3] = node->root->data->at(it->first, 15, lowerBound, upperBound);
                    num32 = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));

                    // Check for extended partition type
                    if ((node->root->data->at(it->first, 4, lowerBound, upperBound) == 0x05) ||
                        (node->root->data->at(it->first, 4, lowerBound, upperBound) == 0x0f)) {
                        AddressNodeList new_nodes = handleExtended(indexPair.first, node);
                        new_children.insert(new_children.end(), new_nodes.begin(), new_nodes.end());
                        /*for (auto it = temp.contentdata.begin(); it !=
                        temp.contentdata.end(); ++it) { blockSeqences.push_back(*it);
                        }
                        for (auto it2 = temp.metadata.begin(); it2 != temp.metadata.end();
                        ++it2) { metadataSeqences.push_back(*it2);
                        }*/
                    }
                    else {
                        indexPair.second = ((num32 - 1) * 512) + indexPair.first;
                        blockSeqence.push_back(indexPair);
                        indexPair.first = 0;
                        indexPair.second = 0;
                        blockSeqences.push_back(blockSeqence);
                        metadataSeqence.push_back(std::make_pair(it->first, it->second));
                        metadataSeqences.push_back(metadataSeqence);

                        //create new node for every partition
                        AddressNodePtr node = std::make_shared<reconstructionNode>(blockSeqences, metadataSeqences, tag);
                        new_children.push_back(node);
                        // result.push_back(node);


                        blockSeqence.clear();
                        blockSeqences.clear();
                        metadataSeqence.clear();
                        metadataSeqences.clear();
                    }
                }
            }
            node->add_children(new_children);
            // output.contentdata = blockSeqences;
            // output.metadata = metadataSeqences;
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }

        result.insert(result.end(), new_children.begin(), new_children.end());
        new_children.clear();
    }
    return result;
}

AddressNodeList DOS::handleExtended(uint64_t offset, AddressNodePtr input)
{
    // Result vector
    std::vector<std::pair<uint64_t, uint64_t>> blockSeqence;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> blockSeqences;
    std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
    std::vector<std::shared_ptr<AddressNode>> output;
    std::string tag = "DOS";
    // Helper
    // LowerBound also functions as index offset to lowest input level
    // TODO Use more than first object
    // TODO Handle fragmentation
    uint64_t lowerBound = input->m_data.front().front().first;
    uint64_t upperBound = input->m_data.back().back().second;
    uint64_t indexOffset = input->m_data.front().front().first;
    std::pair<uint64_t, uint64_t> indexPair;

    try {
        // Variable initialization
        unsigned char bytes[4];
        uint32_t num32 = 0;

        // Seperate partition table entries
        std::vector<std::pair<uint32_t, uint32_t>> tableEntries;
        tableEntries.push_back(std::make_pair(indexOffset + offset + 446, offset + 461));
        tableEntries.push_back(std::make_pair(indexOffset + offset + 462, offset + 477));
        tableEntries.push_back(std::make_pair(indexOffset + offset + 478, offset + 493));
        tableEntries.push_back(std::make_pair(indexOffset + offset + 494, offset + 509));

        for (auto it = tableEntries.begin(); it != tableEntries.end(); ++it) {
            if ((input->root->data->at(it->first, 12, lowerBound, upperBound) == 0x00) &&
                (input->root->data->at(it->first, 13, lowerBound, upperBound) == 0x00) &&
                (input->root->data->at(it->first, 14, lowerBound, upperBound) == 0x00) &&
                (input->root->data->at(it->first, 15, lowerBound, upperBound) == 0x00)) {
                // Skip unused EBR entries    
                break;
            }
            else {
                // Store starting LBA address in sector
                bytes[0] = input->root->data->at(it->first, 8, lowerBound, upperBound);
                bytes[1] = input->root->data->at(it->first, 9, lowerBound, upperBound);
                bytes[2] = input->root->data->at(it->first, 10, lowerBound, upperBound);
                bytes[3] = input->root->data->at(it->first, 11, lowerBound, upperBound);
                num32 = 0;
                std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));
                indexPair.first = indexOffset + (num32 * 512) + offset;

                // Store size in sectors
                bytes[0] = input->root->data->at(it->first, 12, lowerBound, upperBound);
                bytes[1] = input->root->data->at(it->first, 13, lowerBound, upperBound);
                bytes[2] = input->root->data->at(it->first, 14, lowerBound, upperBound);
                bytes[3] = input->root->data->at(it->first, 15, lowerBound, upperBound);
                num32 = 0;
                std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num32));

                // Check for extended partition type
                if ((input->root->data->at(it->first, 4, lowerBound, upperBound) == 0x05) ||
                    (input->root->data->at(it->first, 4, lowerBound, upperBound) == 0x0f)) {
                    AddressNodeList new_nodes = handleExtended(indexPair.first, input);
                    output.insert(output.end(), new_nodes.begin(), new_nodes.end());
                    /*for (auto it = temp.contentdata.begin(); it !=
                    temp.contentdata.end();
                         ++it) {
                      blockSeqences.push_back(*it);
                    }
                    for (auto it2 = temp.metadata.begin(); it2 != temp.metadata.end();
                         ++it2) {
                      metadataSeqences.push_back(*it2);
                    }*/
                }
                else {
                    indexPair.second = ((num32 - 1) * 512) + indexPair.first;
                    blockSeqence.push_back(indexPair);
                    blockSeqences.push_back(blockSeqence);
                    indexPair.first = 0;
                    indexPair.second = 0;
                    metadataSeqence.push_back(std::make_pair(it->first, it->second));
                    metadataSeqences.push_back(metadataSeqence);
                    output.push_back(std::make_shared<reconstructionNode>(blockSeqences, metadataSeqences, tag));
                }

                blockSeqence.clear();
                blockSeqences.clear();
                metadataSeqence.clear();
                metadataSeqences.clear();
            }
        }
        // output.contentdata = blockSeqences;
        // output.metadata = metadataSeqences;
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return output;
}
