#include "ntfs_fsl.h"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <bitset>

Ntfs_fsl::Ntfs_fsl() {}

Ntfs_fsl::~Ntfs_fsl() {}

AddressNodeList Ntfs_fsl::evaluate(AddressNodeList input)
{
    //Result vector
    std::vector<std::pair<uint64_t, uint64_t>> contentSeqence;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> contentSeqences;
    std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
    std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence2;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
    AddressNodeList output;
    AddressNodeList new_children;
    std::string tag = "ntfs_fsl";

    for (AddressNodePtr &node : input) {
        //Helper
        //LowerBound also functions as index offset to lowest input level
        uint64_t lowerBound = node->m_data.front().front().first;
        uint64_t upperBound = node->m_data.back().back().second;
        uint64_t indexOffset = node->m_data.front().front().first;
        std::pair<uint64_t, uint64_t> indexPair;

        try {

            //Variable initialization
            unsigned char bytes[8];
            uint64_t num = 0;
            uint16_t mftEntrySize = 1024;
            uint64_t totalSectorRange;
            uint16_t clusterSize;
            uint64_t firstClusterMFT;
            uint16_t sectorSize;

            //Store bytes per sector
            bytes[0] = node->root->data->at(indexOffset, 11, lowerBound, upperBound);
            bytes[1] = node->root->data->at(indexOffset, 12, lowerBound, upperBound);
            num = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
            sectorSize = num;

            //Store boot sector
            indexPair = std::make_pair(0, 511);
            metadataSeqence.push_back(indexPair);
            metadataSeqences.push_back(metadataSeqence);
            metadataSeqence.clear();

            //Store total number of sectors
            bytes[0] = node->root->data->at(indexOffset, 40, lowerBound, upperBound);
            bytes[1] = node->root->data->at(indexOffset, 41, lowerBound, upperBound);
            bytes[2] = node->root->data->at(indexOffset, 42, lowerBound, upperBound);
            bytes[3] = node->root->data->at(indexOffset, 43, lowerBound, upperBound);
            bytes[4] = node->root->data->at(indexOffset, 44, lowerBound, upperBound);
            bytes[5] = node->root->data->at(indexOffset, 45, lowerBound, upperBound);
            bytes[6] = node->root->data->at(indexOffset, 46, lowerBound, upperBound);
            bytes[7] = node->root->data->at(indexOffset, 47, lowerBound, upperBound);
            num = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&num));
            totalSectorRange = num - 1;

            //Store sectors per cluster
            bytes[0] = node->root->data->at(indexOffset, 13, lowerBound, upperBound);
            num = bytes[0];
            clusterSize = num * sectorSize;

            //Store first cluster of MFT
            bytes[0] = node->root->data->at(indexOffset, 48, lowerBound, upperBound);
            bytes[1] = node->root->data->at(indexOffset, 49, lowerBound, upperBound);
            bytes[2] = node->root->data->at(indexOffset, 50, lowerBound, upperBound);
            bytes[3] = node->root->data->at(indexOffset, 51, lowerBound, upperBound);
            bytes[4] = node->root->data->at(indexOffset, 52, lowerBound, upperBound);
            bytes[5] = node->root->data->at(indexOffset, 53, lowerBound, upperBound);
            bytes[6] = node->root->data->at(indexOffset, 54, lowerBound, upperBound);
            bytes[7] = node->root->data->at(indexOffset, 55, lowerBound, upperBound);
            num = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&num));
            firstClusterMFT = num;

            //MFT entry 0 = MFT entry itself
            uint64_t startMFT = firstClusterMFT * clusterSize;
            uint64_t endMFT = startMFT + mftEntrySize;
            std::vector<unsigned char> mft(node->root->data->copy2Vector(startMFT, endMFT));

            //Evaluate $MFT $Data
            uint64_t mft_size = 0;  //@DM
            std::vector<unsigned char>::iterator it;
            bool dataFound = false;
            while (!dataFound) {
                it = std::find(mft.begin(), mft.end(), 128);
                // extract actual size of attribute content (actually used space of MFT)
                std::copy((it + 48), (it + 55), reinterpret_cast<unsigned char*>(&mft_size)); //@DM
                it = it + 4;
                bytes[0] = *it;
                num = bytes[0];

                if (num != 0) {
                    dataFound = true;
                    it = it - 4;
                }
                else {
                    mft.erase(mft.begin(), it);
                }
            }

            it = it + 64;   //skip attribute
            bytes[0] = *it;
            //Evaluate runOffset and runLength bytes
            uint16_t lower_nibble = bytes[0] & 0xf;       //Run length
            uint16_t upper_nibble = bytes[0] >> 4;        //Run offset

            //Initialize bytes
            bytes[0] = 0x00;
            bytes[1] = 0x00;
            bytes[2] = 0x00;
            bytes[3] = 0x00;
            bytes[4] = 0x00;
            bytes[5] = 0x00;
            bytes[6] = 0x00;
            bytes[7] = 0x00;

            //Get run length
            for (uint16_t i = 0; i < lower_nibble; i++) {
                it += 1;
                bytes[i] = *it;
            }

            uint64_t runLength = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&runLength));

            //Initialize bytes
            bytes[0] = 0x00;
            bytes[1] = 0x00;
            bytes[2] = 0x00;
            bytes[3] = 0x00;
            bytes[4] = 0x00;
            bytes[5] = 0x00;
            bytes[6] = 0x00;
            bytes[7] = 0x00;

            //Get run offset
            for (uint16_t i = 0; i < upper_nibble; i++) {
                it += 1;
                bytes[i] = *it;
            }

            uint64_t runOffset = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&runOffset));

            //Store MFT
            startMFT = runOffset * clusterSize;
            endMFT = startMFT + (runLength * clusterSize) - 1;
            indexPair = std::make_pair(startMFT, endMFT);
            metadataSeqence.push_back(indexPair);
            metadataSeqences.push_back(metadataSeqence);
            metadataSeqence.clear();

            //Store data area
            indexPair = std::make_pair(0, totalSectorRange * sectorSize - 1);
            contentSeqence.push_back(indexPair);
            contentSeqences.push_back(contentSeqence);
            contentSeqence.clear();

            AddressNodePtr new_child = std::make_shared<reconstructionNode>(contentSeqences, metadataSeqences, tag);
            new_children.push_back(new_child);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        node->add_children(new_children);
    }
    output.insert(output.end(), new_children.begin(), new_children.end());
    new_children.clear();    

    return output;
}
