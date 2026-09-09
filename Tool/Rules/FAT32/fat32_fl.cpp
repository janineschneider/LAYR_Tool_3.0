#include "fat32_fl.h"

#include <algorithm>
#include <bitset>
#include <iostream>
#include <memory>
#include <vector>

FAT32_fl::FAT32_fl() {}

FAT32_fl::~FAT32_fl() {}

/**
 * @brief reconstruct the file layer from the input nodes
 *
 * @param input list containing one or more AddressNodes to reconstruct files from. Only one node will be computed at a time
 * @return AddressNodeList contains the reconstructionNodes of the files that were reconstructed
 */
AddressNodeList FAT32_fl::evaluate(AddressNodeList input)
{
  this->m_rawData = input[0]->root->data;
  for (AddressNodePtr node : input) {
    AddressNodeList new_children;

    this->input = node;

    unsigned char bytes[26];
    uint32_t num32 = 0;
    uint16_t num16 = 0;
    uint8_t num8 = 0;

    /*
     * FAT entries
     */

    std::vector<unsigned char> bootSector =
      m_rawData->copy2Vector(node->m_metadata.front().front().first,
        node->m_metadata.front().front().second);
    // Extract sector size (bytes per sector)
    bytes[0] = bootSector.at(11);
    bytes[1] = bootSector.at(12);
    num16 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num16));
    sectorSize = num16;

    // Store cluster size (sectors per cluster)
    num8 = bootSector.at(13);
    // Convert to bytes per cluster
    clusterSize = num8 * sectorSize;

    // Number of FATs
    num8 = bootSector.at(16);
    uint8_t numberOfFATs = num8;

    // Size of FAT
    bytes[0] = bootSector.at(36);
    bytes[1] = bootSector.at(37);
    bytes[2] = bootSector.at(38);
    bytes[3] = bootSector.at(39);
    num32 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
      reinterpret_cast<unsigned char*>(&num32));
    uint32_t fatSize = num32;

    // Extract size of reserved area (in sectors)
    bytes[0] = bootSector.at(14);
    bytes[1] = bootSector.at(15);
    num16 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t),
      reinterpret_cast<unsigned char*>(&num16));
    std::vector<uint16_t> reservedArea;
    reservedArea.push_back(0);
    reservedArea.push_back(num16 - 1);

    // Fat ranges
    std::vector<std::vector<uint32_t>> fatRange;
    std::vector<uint32_t> fat;
    fat.push_back(reservedArea.at(1) + 1);
    fat.push_back(fatSize + reservedArea.at(1));
    fatRange.push_back(fat);
    fat.clear();
    for (uint8_t i = 1; i < numberOfFATs; i++) {
      fat.push_back(fatRange.at(i - 1).at(1) + 1);
      fat.push_back(fatSize + fat.at(0) - 1);
      fatRange.push_back(fat);
      fat.clear();
    }

    fatEntries = evaluateFAT(fatRange);

    /*
     * Root Directory
     */

     // Store root directory start location (cluster)
    bytes[0] = bootSector.at(44);
    bytes[1] = bootSector.at(45);
    bytes[2] = bootSector.at(46);
    bytes[3] = bootSector.at(47);
    num32 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
      reinterpret_cast<unsigned char*>(&num32));
    uint32_t rootDirectory = num32;

    // Root Directory start (sectors)
    std::vector<uint32_t> rootDirectoryRange;
    rootDirectoryRange.push_back(cluster2Sector(rootDirectory, fatRange));

    // Get root directory fat entry
    auto it1 = std::find_if(fatEntries.begin(), fatEntries.end(),
      [](auto& entry) { return entry.at(0).first == 2; });

    // Get root directory lenght
    auto rootPos = *it1;
    uint32_t range = (rootPos.size() * 8) - 1;

    // Store root directory end (sectors)
    rootDirectoryRange.push_back(cluster2Sector(rootDirectory, fatRange) +
      range);

    // Store root directory
    rootDirectoryStart = rootDirectoryRange.at(0) * sectorSize;
    uint32_t rootDirectoryEnd = rootDirectoryRange.at(1) * sectorSize;
    std::vector<unsigned char> rootDirectoryData =
      m_rawData->copy2Vector(rootDirectoryStart, rootDirectoryEnd);

    /*
     * Extract Files
     */

     // Helper variables
    bool deleted = false;
    uint64_t num = 0;
    uint64_t position = 0;
    bool lfn = false;
    uint32_t numberOfLFNEntries = 0;

    while (position <= rootDirectoryData.size()) {
      if ((rootDirectoryData.at(position) == 0x00) &&
        (rootDirectoryData.at(position + 1) == 0x00) &&
        (rootDirectoryData.at(position + 2) == 0x00) &&
        (rootDirectoryData.at(position + 3) == 0x00) &&
        (rootDirectoryData.at(position + 4) == 0x00)) {
        break;
      }
      else {
        // save only start of meta data block. Dummy 0 for end
        indexPair = std::make_pair(rootDirectoryStart + position, 0);
        metadataSeqence.push_back(indexPair);

        // Check for long file name entry
        if (rootDirectoryData.at(position + 11) == 0x0f) {
          lfn = true;
        }

        // Set deleted true or false
        if (rootDirectoryData.at(position) == 0xe5) {
          deleted = true;
        }

        if (lfn) {
          // Get lenght of complete lfn entry
          if (!deleted) {
            numberOfLFNEntries = rootDirectoryData.at(position) & 0xf;
          }
          else {
            // In case of deleted number of lfn entries has to be calculated by
            // number of lfn markers because sequnece number is used to show
            // allocation status
            numberOfLFNEntries = 1;
            uint64_t position2 = position;
            while (true) {
              if (rootDirectoryData.at(position2 + 11 + 32) == 0x0f) {
                numberOfLFNEntries++;
                position2 += 32;
              }
              else {
                break;
              }
            }
          }

          while (numberOfLFNEntries > 0) {
            numberOfLFNEntries--;
            position += 32;
          }
        }

        // Store attribute
        std::string attributes =
          evaluateAttributes(rootDirectoryData.at(position + 11));

        // Store file size
        bytes[0] = rootDirectoryData.at(position + 28);
        bytes[1] = rootDirectoryData.at(position + 29);
        bytes[2] = rootDirectoryData.at(position + 30);
        bytes[3] = rootDirectoryData.at(position + 31);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
          reinterpret_cast<unsigned char*>(&num));
        uint64_t fileSize = num;

        // Store start cluster
        bytes[3] = rootDirectoryData.at(position + 21);
        bytes[2] = rootDirectoryData.at(position + 20);
        bytes[1] = rootDirectoryData.at(position + 27);
        bytes[0] = rootDirectoryData.at(position + 26);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
          reinterpret_cast<unsigned char*>(&num));
        startCluster = num;

        // Store sectors
        uint64_t byteCounter = 0;
        uint64_t sector = 0;
        uint64_t numberOfSectors = 0;

        if (deleted) {
          sector = (((startCluster - 2) * clusterSize) / sectorSize) +
            (node->m_data.front().front().first / 512);
          addDataPair(sector);

          while (byteCounter <= fileSize) {
            byteCounter += clusterSize;

            if (byteCounter > fileSize) {
              numberOfSectors =
                (clusterSize - (byteCounter - fileSize)) / sectorSize;
              for (uint64_t i = 1; i <= numberOfSectors; i++) {
                sector++;
                addDataPair(sector);
              }
            }
            else {
              numberOfSectors = clusterSize / sectorSize;
              for (uint64_t i = 0; i < numberOfSectors; i++) {
                sector++;
                addDataPair(sector);
              }
            }
          }
          byteCounter = 0;
        }
        else {
          for (std::vector<std::vector<std::pair<uint32_t, int32_t>>>::
            const_iterator it = fatEntries.begin();
            it != fatEntries.end(); ++it) {
            if (it->front().first == startCluster) {
              for (std::vector<std::pair<uint32_t, int32_t>>::const_iterator
                it2 = it->begin();
                it2 != it->end(); ++it2) {

                sector = (((it2->first - 2) * clusterSize) / sectorSize) +
                  (node->m_data.front().front().first / 512);
                byteCounter += clusterSize;

                addDataPair(sector);

                if (byteCounter > fileSize) {
                  numberOfSectors =
                    (clusterSize - (byteCounter - fileSize)) / sectorSize;
                  for (uint64_t i = 1; i <= numberOfSectors; i++) {
                    sector++;
                    addDataPair(sector);
                  }
                }
                else {
                  numberOfSectors = clusterSize / sectorSize;
                  for (uint64_t i = 1; i < numberOfSectors; i++) {
                    sector++;
                    addDataPair(sector);
                  }
                }
              }
              break;
            }
          }
          byteCounter = 0;
        }

        dataSeqences.push_back(dataSeqence);

        // Add end of metadata
        metadataSeqence.back().second = rootDirectoryStart + position + 31;
        metadataSeqences.push_back(metadataSeqence);

        if (attributes.find("Directory") != attributes.npos) {
          handleDirectories();
        }
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> temp_data, temp_metadata;
        temp_data.push_back(dataSeqence);
        temp_metadata.push_back(metadataSeqence);
        AddressNodePtr new_node = std::make_shared<reconstructionNode>(temp_data, temp_metadata, tag);
        new_children.push_back(new_node);
        metadataSeqence.clear();
        dataSeqence.clear();
        position += 32;
        deleted = false;
        lfn = false;
      }
    }
    //new_children.push_back(new_node);
    node->add_children(new_children);
    output.insert(output.end(), new_children.begin(), new_children.end());
  }
  return output;
}

uint32_t FAT32_fl::cluster2Sector(uint32_t cluster,
  std::vector<std::vector<uint32_t>> fatRange)
{
  uint32_t sectorsPerCluster = clusterSize / sectorSize;
  uint32_t lastSectorFAT = fatRange.back().back();

  uint32_t number = (lastSectorFAT + 1) +
    ((((cluster) & 0x0fffffff) - 2) * sectorsPerCluster);

  return number;
}

std::vector<std::vector<std::pair<uint32_t, int32_t>>>
FAT32_fl::evaluateFAT(std::vector<std::vector<uint32_t>> fatRange)
{

  std::vector<unsigned char> m_fat;
  if (clusterSize != 1) {
    // Mirrors
    m_fat = m_rawData->copy2Vector(fatRange.front().at(0) * sectorSize,
      (fatRange.front().at(1) * sectorSize) - 1);
  }
  else {
    // Only the first FAT is active
  }

  unsigned char bytes[4];
  uint32_t num32;
  std::vector<int32_t> fatEntries; // index, entry ID, next cluster

  for (size_t i = 0; i < m_fat.size(); i += 4) {
    // Store entry ID = byte offset / 4 bytes per entry
    if (i == 0) {
      fatEntries.push_back(0);
    }
    else {
      fatEntries.push_back(i / 4);
    }

    // Store next cluster
    bytes[0] = m_fat.at(i);
    bytes[1] = m_fat.at(i + 1);
    bytes[2] = m_fat.at(i + 2);
    bytes[3] = m_fat.at(i + 3);
    num32 = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
      reinterpret_cast<unsigned char*>(&num32));

    // 0 = not allocated
    //-1 = damaged cluster
    if (num32 == 268435447) {
      fatEntries.push_back(-1);
    }
    //-2 = EOF
    else if (num32 >= 268435448) {
      fatEntries.push_back(-2);
    }
    // Else next cluster
    else {
      fatEntries.push_back(num32);
    }
  }

  int32_t helper;
  std::vector<std::vector<std::pair<uint32_t, int32_t>>> clusterChains;
  std::vector<std::pair<uint32_t, int32_t>> clusterChain;

  // Start with entry 2 (the first 2 entries are reserved)
  for (uint32_t i = 4; i < fatEntries.size(); i += 2) {
    if ((fatEntries.at(i + 1) > 0) || (fatEntries.at(i + 1) == -2)) {
      helper = fatEntries.at(i + 1);
      clusterChain.push_back(
        std::make_pair(fatEntries.at(i), fatEntries.at(i + 1)));
      fatEntries.at(i + 1) = 0;
      uint32_t j = i;
      while (helper != -2) {
        // Find next cluster
        auto it =
          std::find(fatEntries.begin() + j + 2, fatEntries.end(), helper);
        if (it != fatEntries.end()) {
          clusterChain.push_back(std::make_pair(*it, *(it + 1)));
          helper = *(it + 1);
          // Assign 0 for sorting purposes
          *(it + 1) = 0;
          j = it - fatEntries.begin();
        }
        else {
          break;
        }
      }
      clusterChains.push_back(clusterChain);
      clusterChain.clear();
    }
  }
  return clusterChains;
}

std::string FAT32_fl::evaluateAttributes(unsigned char hex)
{
  uint8_t dec = hex;
  std::string attributes = "";

  std::bitset<8> bits(dec);
  for (size_t i = 0; i < bits.size(); i++) {
    if (bits.test(i)) {
      if (i == 0)
        attributes.append("Read only, ");

      if (i == 1)
        //                attributes.append("Hidden file, ");
        attributes.append("Hidden, ");

      if (i == 2)
        //                attributes.append("System file, ");
        attributes.append("System, ");

      if (i == 3)
        //                attributes.append("Volume label, ");
        attributes.append("Volume, ");

      if (i == 4)
        attributes.append("Directory, ");

      if (i == 5)
        attributes.append("Archive, ");
    }
    else if (!bits.test(i)) {
      if (i == 4)
        attributes.append("File, ");
    }
  }

  attributes.pop_back();
  attributes.pop_back();

  return attributes;
}

void FAT32_fl::addDataPair(uint64_t sector)
{
  // Adapt only end of last entry, because it is continues
  if (!dataSeqence.empty() &&
    dataSeqence.back().second == sector * sectorSize - 1) {
    dataSeqence.back().second = sector * sectorSize + (sectorSize - 1);
    // Fragmented
  }
  else {
    indexPair = std::make_pair(sector * sectorSize,
      sector * sectorSize + (sectorSize - 1));
    dataSeqence.push_back(indexPair);
  }
}

void FAT32_fl::handleDirectories()
{

  //+ 64 because . and .. shall be skipped
  uint64_t start = (startCluster - 2) * clusterSize + 64;
  uint64_t end = (start + clusterSize) - 1;

  std::vector<unsigned char> directoryEntry =
    m_rawData->copy2Vector(start, end);
  std::vector<uint32_t> sectors;

  // Variable initialization
  unsigned char bytes[26];
  uint64_t num = 0;

  bool lfn = false;
  uint64_t position = 0;
  bool deleted = false;
  uint64_t numberOfLFNEntries = 0;

  while (position <= directoryEntry.size()) {
    if ((directoryEntry.at(position) == 0x00) &&
      (directoryEntry.at(position + 1) == 0x00) &&
      (directoryEntry.at(position + 2) == 0x00) &&
      (directoryEntry.at(position + 3) == 0x00) &&
      (directoryEntry.at(position + 4) == 0x00)) {
      break;
    }
    else {

      // save only start of meta data block. Dummy 0 for end
      indexPair = std::make_pair(rootDirectoryStart + position, 0);
      metadataSeqence.push_back(indexPair);

      // Check for long file name entry
      if (directoryEntry.at(position + 11) == 0x0f) {
        lfn = true;
      }

      // Set deleted true or false
      if (directoryEntry.at(position) == 0xe5) {
        deleted = true;
      }

      if (lfn) {
        // Get lenght of complete lfn entry
        if (!deleted) {
          numberOfLFNEntries = directoryEntry.at(position) & 0xf;
        }
        else {
          // In case of deleted number of lfn entries has to be calculated by
          // number of lfn markers because sequnece number is used to show
          // allocation status
          numberOfLFNEntries = 1;
          uint64_t position2 = position;
          while (true) {
            if (directoryEntry.at(position2 + 11 + 32) == 0x0f) {
              numberOfLFNEntries++;
              position2 += 32;
            }
            else {
              break;
            }
          }
        }

        while (numberOfLFNEntries > 0) {
          numberOfLFNEntries--;
          position += 32;
        }
      }

      // Store attribute
      std::string attributes =
        evaluateAttributes(directoryEntry.at(position + 11));

      // Store file size
      bytes[0] = directoryEntry.at(position + 28);
      bytes[1] = directoryEntry.at(position + 29);
      bytes[2] = directoryEntry.at(position + 30);
      bytes[3] = directoryEntry.at(position + 31);
      num = 0;
      std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
        reinterpret_cast<unsigned char*>(&num));
      uint64_t fileSize = num;

      // Store start cluster
      bytes[3] = directoryEntry.at(position + 21);
      bytes[2] = directoryEntry.at(position + 20);
      bytes[1] = directoryEntry.at(position + 27);
      bytes[0] = directoryEntry.at(position + 26);
      num = 0;
      std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t),
        reinterpret_cast<unsigned char*>(&num));
      uint64_t startCluster = num;

      // Store sectors
      uint64_t byteCounter = 0;
      uint64_t sector = 0;
      uint64_t numberOfSectors = 0;

      if (deleted) {

        sector = (((startCluster - 2) * clusterSize) / sectorSize) +
          (input->m_data.front().front().first / 512);
        addDataPair(sector);

        while (byteCounter <= fileSize) {
          byteCounter += clusterSize;

          if (byteCounter > fileSize) {
            numberOfSectors =
              (clusterSize - (byteCounter - fileSize)) / sectorSize;
            for (uint64_t i = 1; i <= numberOfSectors; i++) {
              sector++;
              addDataPair(sector);
            }
          }
          else {
            numberOfSectors = clusterSize / sectorSize;
            for (uint64_t i = 0; i < numberOfSectors; i++) {
              sector++;
              addDataPair(sector);
            }
          }
        }
        byteCounter = 0;

      }
      else {
        for (std::vector<
          std::vector<std::pair<uint32_t, int32_t>>>::const_iterator it =
          fatEntries.begin();
          it != fatEntries.end(); ++it) {
          if (it->front().first == startCluster) {
            for (std::vector<std::pair<uint32_t, int32_t>>::const_iterator it2 =
              it->begin();
              it2 != it->end(); ++it2) {

              sector = (((it2->first - 2) * clusterSize) / sectorSize) +
                (input->m_data.front().front().first / 512);

              byteCounter += clusterSize;

              if (byteCounter > fileSize) {
                numberOfSectors =
                  (clusterSize - (byteCounter - fileSize)) / sectorSize;
                for (uint64_t i = 1; i <= numberOfSectors; i++) {
                  sector++;
                  addDataPair(sector);
                }
              }
              else {
                numberOfSectors = clusterSize / sectorSize;
                for (uint64_t i = 1; i < numberOfSectors; i++) {
                  sector++;
                  addDataPair(sector);
                }
              }
            }
            break;
          }
        }
      }

      dataSeqences.push_back(dataSeqence);
      dataSeqence.clear();
      byteCounter = 0;

      // Add end of metadata
      metadataSeqence.back().second = rootDirectoryStart + position + 31;
      metadataSeqences.push_back(metadataSeqence);
      metadataSeqence.clear();

      if (attributes.find("Directory") != attributes.npos) {
        handleDirectories();
      }

      position += 32;
      lfn = false;
      deleted = false;
    }
  }
}
