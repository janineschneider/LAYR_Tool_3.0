#include "ntfs_fl.h"

#include <iostream>
#include <algorithm>

Ntfs_fl::Ntfs_fl() {}
Ntfs_fl::~Ntfs_fl() {}

AddressNodeList Ntfs_fl::evaluate(AddressNodeList input)
{
    for (AddressNodePtr& node : input) {
        //Helper
        uint64_t lowerBoundBootSector = node->m_metadata.front().front().first;
        uint64_t upperBoundBootSector = node->m_metadata.front().front().second;
        uint64_t lowerBoundMFT = node->m_metadata.back().front().first;
        uint64_t upperBoundMFT = node->m_metadata.back().front().second;

        try {
            //Variable initialization
            unsigned char bytes[256];

            uint64_t num = 0;

            uint32_t entryCounter = 0; //@DM4
            uint64_t position = 0;

            uint64_t startAttribute = 0;

            //Store MFT
            std::vector<unsigned char> mftData = node->root->data->copy2Vector(lowerBoundMFT, upperBoundMFT);

            //Store bytes per sector
            bytes[0] = node->root->data->at(11, lowerBoundBootSector, upperBoundBootSector);
            bytes[1] = node->root->data->at(12, lowerBoundBootSector, upperBoundBootSector);
            num = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
            uint16_t sectorSize = num;

            //Store sectors per cluster
            bytes[0] = node->root->data->at(13, lowerBoundBootSector, upperBoundBootSector);

            num = bytes[0];
            uint16_t clusterSize = num * sectorSize;

            //Store total number of sectors
            bytes[0] = node->root->data->at(40, lowerBoundBootSector, upperBoundBootSector);
            bytes[1] = node->root->data->at(41, lowerBoundBootSector, upperBoundBootSector);
            bytes[2] = node->root->data->at(42, lowerBoundBootSector, upperBoundBootSector);
            bytes[3] = node->root->data->at(43, lowerBoundBootSector, upperBoundBootSector);
            bytes[4] = node->root->data->at(44, lowerBoundBootSector, upperBoundBootSector);
            bytes[5] = node->root->data->at(45, lowerBoundBootSector, upperBoundBootSector);
            bytes[6] = node->root->data->at(46, lowerBoundBootSector, upperBoundBootSector);
            bytes[7] = node->root->data->at(47, lowerBoundBootSector, upperBoundBootSector);
            num = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&num));
            uint64_t totalSectorRange = num - 1;

            //Store total number of clusters
            uint64_t totalClusterRange = ((totalSectorRange * sectorSize) / clusterSize) - 1;
            contentSeqence.clear();

            while (position < mftData.size()) {
                if ((mftData.at(position) == 'F') &&
                    (mftData.at(position + 1) == 'I') &&
                    (mftData.at(position + 2) == 'L') &&
                    (mftData.at(position + 3) == 'E')) {

                    //MFT entry is not empty
                    if (isMftEmpty(&mftData, position)) {
                        uint64_t mftEntryNum = position / 1024;
                        std::string currentFilename = "";

                        //Store MFT Entry in metadata
                        metadataSeqence.push_back(std::make_pair(lowerBoundMFT + position, lowerBoundMFT + position + 1023));
                        metadataSeqences.push_back(metadataSeqence);
                        metadataSeqence.clear();

                        /*
                        * Evaluate MFT entry attributes
                        */
                        //Set starting position of first attribute
                        bytes[0] = mftData.at(position + 20);
                        bytes[1] = mftData.at(position + 21);
                        num = 0;
                        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
                        startAttribute = position + num;

                        bool nonResident = false;
                        std::string attrib_name = "";

                        while (true) {
                            //Check for end of attributes
                            bytes[0] = mftData.at(startAttribute);
                            bytes[1] = mftData.at(startAttribute + 1);
                            bytes[2] = mftData.at(startAttribute + 2);
                            bytes[3] = mftData.at(startAttribute + 3);

                            if ((bytes[0] == 0xff) && (bytes[1] == 0xff) && (bytes[2] == 0xff) && (bytes[3] == 0xff)) {
                                break;
                            }

                            //start @DM8
                            //Get attribute type identifier
                            num = 0;
                            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
                            uint32_t attributeTypeIdentifier = num;
                            //end @DM8

                            //start @DM7 get name of attribute
                            uint8_t name_length = 0;
                            bytes[0] = mftData.at(startAttribute + 9);
                            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint8_t), reinterpret_cast<unsigned char*>(&name_length));

                            if (name_length > 0) {
                                int name_offset = 0;
                                uint16_t unicode;
                                bytes[0] = mftData.at(startAttribute + 10);
                                bytes[1] = mftData.at(startAttribute + 11);
                                std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&name_offset));
                                for (int i = 0; i < name_length * 2; i += 2) { //16-bit names in ntfs
                                    std::copy(&(mftData.at(startAttribute + (name_offset + i))), &(mftData.at(startAttribute + (name_offset + i))) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&unicode));
                                    attrib_name = attrib_name + static_cast<char>(unicode);
                                }
                            }
                            //end @DM7

                            //Get resident flag
                            bytes[0] = mftData.at(startAttribute + 8);
                            //144 = $INDEX_ROOT = always resident
                            if ((bytes[0] != 0x00) && (attributeTypeIdentifier != 144)) {
                                nonResident = true;
                            }

                            //Handle $FILE_NAME attribute
                            if (attributeTypeIdentifier == 48 && !nonResident) {
                                currentFilename = getFileName(&mftData, startAttribute);
                                //TODO what happens if multi hard links resp. $FILE_NAME exists
                            }

                            //Handle $DATA attribute
                            if (attributeTypeIdentifier == 128) {
                                if (!nonResident) {
                                    // Get length of resident data (4 bytes at attribute header offset 16)
                                    bytes[0] = mftData.at(startAttribute + 16);
                                    bytes[1] = mftData.at(startAttribute + 17);
                                    bytes[2] = mftData.at(startAttribute + 18);
                                    bytes[3] = mftData.at(startAttribute + 19);
                                    uint32_t residentDataOffset = 0;
                                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&residentDataOffset));

                                    //Get offset to content of resident $DATA attribute
                                    bytes[0] = mftData.at(startAttribute + 20);
                                    bytes[1] = mftData.at(startAttribute + 21);
                                    uint16_t offset = 0;
                                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(offset), reinterpret_cast<unsigned char*>(&offset));

                                    // Push resident data range
                                    if (residentDataOffset > 0) {
                                        uint64_t startByte = lowerBoundMFT + startAttribute + offset;
                                        uint64_t endByte = startByte + residentDataOffset - 1;
                                        contentSeqence.push_back(std::make_pair(startByte, endByte));
                                    }


                                }
                                else {
                                    //Handle $Bad from $BadClus
                                    //start @DM2
                                    //if (mftData.at(startAttribute + 64) == 0x24) {
                                    if (attrib_name == "$Bad") {
                                        //end @DM2
                                        //start @DM10
                                        uint64_t contentLength = (totalClusterRange + 1) * clusterSize; //1052803072;
                                        //Evaluate attribute size and offset to next attribute
                                        bytes[0] = mftData.at(startAttribute + 4);
                                        bytes[1] = mftData.at(startAttribute + 5);
                                        bytes[2] = 0x00;
                                        bytes[3] = 0x00;
                                        num = 0;
                                        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
                                        uint64_t offset = num;

                                        //Search for bad clusters
                                        uint32_t counter = 0;
                                        while (counter < contentLength - offset) {

                                            //Search for attriubte end
                                            bytes[0] = mftData.at(startAttribute + offset + counter);
                                            bytes[1] = mftData.at(startAttribute + offset + 1 + counter);
                                            bytes[2] = mftData.at(startAttribute + offset + 2 + counter);
                                            bytes[3] = mftData.at(startAttribute + offset + 3 + counter);

                                            if ((bytes[0] == 0xff) && (bytes[1] == 0xff) && (bytes[2] == 0xff) && (bytes[3] == 0xff)) {
                                                break;
                                            }

                                            // Initialize bytes array to prevent stale data
                                            for (int i = 0; i < 8; ++i) {
                                                bytes[i] = 0x00;
                                            }

                                            //Or search for next bad cluster
                                            bytes[0] = mftData.at(startAttribute + offset + counter);
                                            bytes[1] = mftData.at(startAttribute + offset + counter + 1);
                                            bytes[2] = mftData.at(startAttribute + offset + counter + 2);
                                            bytes[3] = mftData.at(startAttribute + offset + counter + 3);
                                            bytes[4] = mftData.at(startAttribute + offset + counter + 4);
                                            bytes[5] = mftData.at(startAttribute + offset + counter + 5);
                                            bytes[6] = mftData.at(startAttribute + offset + counter + 6);
                                            bytes[7] = mftData.at(startAttribute + offset + counter + 7);
                                            num = 0;
                                            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char*>(&num));

                                            //Add bad cluster to contentSequence
                                            uint64_t startCluster = num * clusterSize;
                                            uint64_t endCluster = startCluster + clusterSize - 1;

                                            contentSeqence.push_back(std::make_pair(startCluster, endCluster));

                                            counter += 8;
                                        }
                                        //end @DM10
                                    }
                                    else {
                                        //Evaluate runOffset and runLength bytes
                                        // start @DM2
                                        // extract offset to the runlist
                                        int offset_to_the_runlist = 0;
                                        bytes[0] = mftData.at(startAttribute + 32);
                                        bytes[1] = mftData.at(startAttribute + 33);
                                        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&offset_to_the_runlist));

                                        bytes[0] = mftData.at(startAttribute + offset_to_the_runlist);
                                        // end @DM2

                                        //int64_t for signed LCN deltas
                                        int64_t runOffset = 0;

                                        while (bytes[0] != 0x00) {
                                            uint8_t lower_nibble = bytes[0] & 0x0f;       //Run length
                                            uint8_t upper_nibble = bytes[0] >> 4;        //Run offset

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
                                            for (uint8_t i = 0; i < lower_nibble; i++) {
                                                bytes[i] = mftData.at(startAttribute + (offset_to_the_runlist + 1) + i); //@DM2
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
                                            for (uint8_t i = 0; i < upper_nibble; i++) {
                                                bytes[i] = mftData.at(startAttribute + (offset_to_the_runlist + 1) + lower_nibble + i); //@DM3, @DM2 -> shift bytes by no. of bytes saved in lower_nibble (not always 1!!!)
                                            }

                                            int64_t offset = 0;
                                            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(int64_t), reinterpret_cast<unsigned char*>(&offset));

                                            //sparse
                                            if (upper_nibble > 0 && (bytes[upper_nibble - 1] & 0x80)) {
                                                for (uint8_t i = upper_nibble; i < 8; ++i) {
                                                    reinterpret_cast<unsigned char*>(&offset)[i] = 0xFF;
                                                }
                                            }

                                            // Accumulate LCN delta
                                            runOffset += offset;

                                            //Not sparse
                                            if (upper_nibble > 0) {
                                                uint64_t startCluster = runOffset * clusterSize;
                                                uint64_t endCluster = (runOffset + runLength) * clusterSize - 1;
                                                contentSeqence.push_back(std::make_pair(startCluster, endCluster));
                                            }


                                            offset_to_the_runlist += lower_nibble + upper_nibble + 1;
                                            bytes[0] = mftData.at(startAttribute + offset_to_the_runlist);
                                        }
                                    }
                                }
                            }

                            //Jumpt to next attribute
                            //(Hint: The Length is 4 Bytes, but only the first two bytes should be use.
                            //       If the full 4 Bytes are used, errors occure in some Images. It happens
                            //       e.g. if a full Windows 10 Image is analysed.)
                            bytes[0] = mftData.at(startAttribute + 4);
                            bytes[1] = mftData.at(startAttribute + 5);
                            bytes[2] = 0x00; //See Hint
                            bytes[3] = 0x00; //See Hint

                            num = 0;
                            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));

                            startAttribute += num;

                            nonResident = false;
                            attrib_name = "";
                        }

                        //Store data area
                        contentSeqences.push_back(contentSeqence);
                        contentSeqence.clear();

                        AddressNodePtr new_child = std::make_shared<reconstructionNode>(contentSeqences, metadataSeqences, tag + "(" + currentFilename + ")");
                        new_children.push_back(new_child);
                        contentSeqences.clear();
                        metadataSeqences.clear();
                    }
                }
                entryCounter++; //@DM4
                position += 1024;
            }

            node->add_children(new_children);

        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }

        output.insert(output.end(), new_children.begin(), new_children.end());
        new_children.clear();
    }

    return output;
}

std::string Ntfs_fl::getFileName(std::vector<unsigned char>* mftData, uint64_t attributePosition)
{
    //Variable initialization
    unsigned char bytes[256];
    uint64_t num;
    wchar_t unicode[100];
    std::string name = "";

    //start @DM11
    //Evaluate attribute size and offset to next attribute
    bytes[0] = mftData->at(attributePosition + 4);
    bytes[1] = mftData->at(attributePosition + 5);
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    num = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
    uint32_t offset = num;

    //Evaluate content position
    bytes[0] = mftData->at(attributePosition + 20);
    bytes[1] = mftData->at(attributePosition + 21);
    num = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
    uint64_t content_position = num;

    attributePosition += num;

    //Store $FILE_NAME name
    if (offset - content_position > 64) { //@DM11
        bytes[0] = mftData->at(attributePosition + 64);
        uint8_t nameLength2 = bytes[0];
        nameLength2 = nameLength2 * 2;

        for (uint8_t i = 0; i < nameLength2; i++) {
            bytes[i] = mftData->at(attributePosition + 66 + i);
        }

        nameLength2 = nameLength2 / 2;
        uint8_t odd = 1;
        uint8_t even = 0;
        for (uint8_t i = 0; i < nameLength2; i++) {
            //Convert name to unicode name
            unicode[i] = bytes[odd] << 8;
            unicode[i] = unicode[i] | bytes[even];
            odd += 2;
            even += 2;
        }

        std::wstring unicodeName(unicode);
        std::string nameString(unicodeName.begin(), unicodeName.end());
        if (nameString.size() > nameLength2) {
            nameString.erase(nameLength2);
        }

        name = nameString;
    }
    return name;
}

bool Ntfs_fl::isMftEmpty(std::vector<unsigned char>* mftData, uint64_t position)
{
    uint64_t nameLength = 0;
    unsigned char bytes[4];
    uint32_t num;

    auto it = find(mftData->begin() + position + 42,
        mftData->begin() + position + 1023, 16);
    uint64_t nameLengthPosition = it - mftData->begin();

    if (it != mftData->begin() + position + 1023) {
        //Evaluate attribute size
        bytes[0] = mftData->at(nameLengthPosition + 4);
        bytes[1] = mftData->at(nameLengthPosition + 5);
        bytes[2] = 0x00;
        bytes[3] = 0x00;
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
        uint64_t nameLenghtOffset = num;

        it = find(mftData->begin() + position + 42 + nameLenghtOffset,
            mftData->begin() + position + 1023, 48);
        nameLengthPosition = it - mftData->begin();

        //Is bigger as MFT entry?
        if (nameLengthPosition > 1024) {
            return true;
        }

        if (it != mftData->begin() + position + 1023) {
            //Evaluate content position
            bytes[0] = mftData->at(nameLengthPosition + 20);
            bytes[1] = mftData->at(nameLengthPosition + 21);
            num = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
            nameLengthPosition += num;

            bytes[0] = mftData->at(nameLengthPosition + 64);

            if (nameLengthPosition < position + 1023) {
                nameLength = bytes[0];
            }
        }
    }
    return nameLength > 0;
}
