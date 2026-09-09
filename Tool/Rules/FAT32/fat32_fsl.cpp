#include "fat32_fsl.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

FAT32_fsl::FAT32_fsl() {}

FAT32_fsl::~FAT32_fsl() {}

/**
 * @brief reconstruct the file system layer from the input
 * 
 * @param input list of AddressNodes to reconstruct the file system from. Only one node is used at a time
 * @return AddressNodeList contains the reconstructionNodes for the file systems found 
 */
AddressNodeList
FAT32_fsl::evaluate(AddressNodeList input) {
  AddressNodeList output;

  for (AddressNodePtr node : input) {
    // Result vector
    AddressNodeList new_children;
    std::string tag = "FAT32_fsl";
    std::vector<std::pair<uint64_t, uint64_t>> contentSeqence;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> contentSeqences;
    std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;

    // Helper
    // LowerBound also functions as index offset to lowest input level
    // TODO Use more than first object
    // TODO Handle fragmentation
    uint64_t lowerBound = node->m_data.front().front().first;
    uint64_t upperBound = node->m_data.back().back().second;
    uint64_t indexOffset = node->m_data.front().front().first;
    std::pair<uint64_t, uint64_t> indexPair;

    // Variable initialization
    unsigned char bytes[11];
    uint32_t num32 = 0;
    uint16_t num16 = 0;
    uint8_t num8 = 0;

    /*
     * Boot Sector
     */

    // Store boot sector indices as file system metadata
    uint16_t bootSectorStart = indexOffset + 0;
    uint16_t bootSectorEnd = indexOffset + 511;
    indexPair = std::make_pair(bootSectorStart, bootSectorEnd);
    metadataSeqence.push_back(indexPair);
    metadataSeqences.push_back(metadataSeqence);
    metadataSeqence.clear();

    // Extract size of reserved area (in sectors)
    bytes[0] = node->root->data->at(indexOffset, 14, lowerBound, upperBound);
    bytes[1] = node->root->data->at(indexOffset, 15, lowerBound, upperBound);
    num16 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t),
              reinterpret_cast<unsigned char *>(&num16));
    std::vector<uint16_t> reservedArea;
    reservedArea.push_back(0);
    reservedArea.push_back(num16 - 1);

    // Extract sector size (bytes per sector)
    bytes[0] = node->root->data->at(indexOffset, 11, lowerBound, upperBound);
    bytes[1] = node->root->data->at(indexOffset, 12, lowerBound, upperBound);
    num16 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t),
              reinterpret_cast<unsigned char *>(&num16));
    uint16_t sectorSize = num16;

    // Store total number of sectors (sector range)
    std::vector<uint32_t> totalSectors;
    totalSectors.push_back(0);
    bytes[0] = node->root->data->at(indexOffset, 32, lowerBound, upperBound);
    bytes[1] = node->root->data->at(indexOffset, 33, lowerBound, upperBound);
    bytes[2] = node->root->data->at(indexOffset, 34, lowerBound, upperBound);
    bytes[3] = node->root->data->at(indexOffset, 35, lowerBound, upperBound);
    num32 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
              reinterpret_cast<unsigned char *>(&num32));
    totalSectors.push_back(num32 - 1);

    /*
     * FAT
     */

    // Number of FATs
    num8 = node->root->data->at(indexOffset, 16, lowerBound, upperBound);
    uint8_t numberOfFATs = num8;

    // Size of FAT
    bytes[0] = node->root->data->at(indexOffset, 36, lowerBound, upperBound);
    bytes[1] = node->root->data->at(indexOffset, 37, lowerBound, upperBound);
    bytes[2] = node->root->data->at(indexOffset, 38, lowerBound, upperBound);
    bytes[3] = node->root->data->at(indexOffset, 39, lowerBound, upperBound);
    num32 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
              reinterpret_cast<unsigned char *>(&num32));
    uint32_t fatSize = num32;

    // Store fat indices as file system metadata
    uint32_t fatStart = indexOffset + (reservedArea.at(1) + 1) * sectorSize;
    uint32_t fatEnd =
        indexOffset +
        (reservedArea.at(1) + (numberOfFATs * fatSize) + 1) * sectorSize - 1;
    indexPair = std::make_pair(fatStart, fatEnd);
    metadataSeqence.push_back(indexPair);
    metadataSeqences.push_back(metadataSeqence);
    metadataSeqence.clear();

    /*
     * Data Area
     */

    // Store data area indices as file system metadata
    uint32_t dataAreaStart = indexOffset + fatEnd + 1;
    uint32_t dataAreaEnd =
        indexOffset + (totalSectors.back() + 1) * sectorSize - 1;
    indexPair = std::make_pair(dataAreaStart, dataAreaEnd);
    contentSeqence.push_back(indexPair);
    contentSeqences.push_back(contentSeqence);
    contentSeqence.clear();

    AddressNodePtr new_child = std::make_shared<reconstructionNode>(
        contentSeqences, metadataSeqences, tag);
    // return
    output.push_back(new_child);
    node->add_children(std::vector{new_child});
  }
  return output;
}
