#include "ntfs_fl_dfxml.h"

#include <iostream>
#include <map>
#include <sstream>
#include <cctype>

#include <iostream>
#include <algorithm>
#include <bitset>
#include <cctype>
#include <ctime>

Ntfs_fl_dfxml::Ntfs_fl_dfxml() {}

Ntfs_fl_dfxml::~Ntfs_fl_dfxml() {}

void Ntfs_fl_dfxml::evaluate(AddressNodeList input)
{
    std::set<uint64_t> processed_ids;

    //DFXML Handler
    dfxml_writer outputWriter;
    outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
    outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

    //Iterate inodes
    for (AddressNodePtr &node : input) {
        //Helper
        uint64_t lowerBound = 0;
        uint64_t upperBound = 0;
        uint64_t indexOffset = 0;
        uint64_t id = 0;
        unsigned char bytes[100]; //TODO: site of bytes
        uint64_t num = 0;    
        for (auto it = node->m_metadata.begin(); it != node->m_metadata.end(); ++it) {
            if (processed_ids.find(id) != processed_ids.end()) {
                id++;
                continue; // Jump if this id has already been written
            }
            processed_ids.insert(id);
            for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
                lowerBound = it2->first;
                upperBound = it2->second;
                indexOffset = lowerBound;

                //DFXML
                outputWriter.push("fileobject");
                outputWriter.xmlout("id", id);

                //Set starting position of first attribute
                bytes[0] = node->root->data->at(indexOffset, 20, lowerBound, upperBound);
                bytes[1] = node->root->data->at(indexOffset, 21, lowerBound, upperBound);
                num = 0;
                std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
                uint64_t startAttribute = num;


                while (true) {
                    //Check for end of attributes
                    bytes[0] = node->root->data->at(indexOffset, startAttribute, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(indexOffset, startAttribute + 1, lowerBound, upperBound);
                    bytes[2] = node->root->data->at(indexOffset, startAttribute + 2, lowerBound, upperBound);
                    bytes[3] = node->root->data->at(indexOffset, startAttribute + 3, lowerBound, upperBound);
                    if ((bytes[0] == 0xff) && (bytes[1] == 0xff) && (bytes[2] == 0xff) && (bytes[3] == 0xff)) {
                        break;
                    }

                    //Get attribute type identifier
                    num = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
                    uint32_t attributeTypeIdentifier = num;

                    //Get resident flag
                    bytes[0] = node->root->data->at(indexOffset, startAttribute + 8, lowerBound, upperBound);
                    bool nonResident = false;
                    if ((bytes[0] != 0x00) && (attributeTypeIdentifier != 144)) nonResident = true;

                    if (attributeTypeIdentifier == 16) {  //Handle $STANDARD_INFORMATION attribute
                        printStandardInformation(node, &outputWriter, startAttribute, indexOffset, lowerBound, upperBound);
                    } else if (attributeTypeIdentifier == 48 && !nonResident) { //Handle $FILE_NAME attribute
                        printFileName(node, &outputWriter, startAttribute, indexOffset, lowerBound, upperBound);
                    }

                    //Jumpt to next attribute
                    bytes[0] = node->root->data->at(indexOffset, startAttribute + 4, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(indexOffset, startAttribute + 5, lowerBound, upperBound);
                    bytes[2] = 0x00;
                    bytes[3] = 0x00;
                    num = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
                    startAttribute += num;
                }

            }
            //Size of file content
            outputWriter.push("byte_runs");
            for (auto it2 = node->m_data.at(id).begin(); it2 != node->m_data.at(id).end(); ++it2) {
                std::vector<std::string> tags = {"fs_offset", "len"};
                std::vector<uint64_t> values = {it2->first, it2->second - it2->first + 1};
                std::string attributes = "";
                for (size_t i = 0; i < tags.size(); i++) {
                    attributes.append(tags.at(i) + "=\"" + std::to_string(values.at(i)) + "\" ");
                }

                outputWriter.xmlout("byte_run", "", attributes, true);
            }
            outputWriter.pop();
            outputWriter.pop();
            id++;
        }
    }
    outputWriter.pop();
}

std::string Ntfs_fl_dfxml::nanoseconds2DateTime(uint64_t nanoseconds) 
{
    uint64_t ticksPerSecond = 10000000;
    uint64_t epochDifference = 11644473600LL;
    uint64_t temp;

    temp = nanoseconds / ticksPerSecond; //convert from 100ns intervals to seconds;
    temp = temp - epochDifference;  //subtract number of seconds between epochs

    time_t t = (time_t)temp;
    struct tm *tm = localtime(&t);
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm);

    std::string dateTime(date);
    dateTime.append(".");

    std::string nanostring = std::to_string(nanoseconds % ticksPerSecond);
    for (size_t i = nanostring.size(); i < 7; i++) {
        dateTime.append("0");
    }

    dateTime.append(nanostring);
    dateTime.append("00");
    //Todo: tm_zone not necessarily included in systems like Windows
    //std::string timezone = " (" + std::string(tm->tm_zone) + ")"; //@DM6
    //dateTime.append(timezone); //@DM6

    return dateTime;
}
void Ntfs_fl_dfxml::printStandardInformation(AddressNodePtr node, dfxml_writer* outputWriter, uint64_t attributePosition, uint64_t indexOffset, uint64_t lowerBound, uint64_t upperBound) {
    //Variable initialization
    unsigned char bytes[256];
    uint64_t num;

    //Evaluate attribute size and offset to next attribute
    bytes[0] = node->root->data->at(indexOffset, attributePosition + 4, lowerBound, upperBound);
    bytes[1] = node->root->data->at(indexOffset, attributePosition + 5, lowerBound, upperBound);
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    num = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
    uint64_t offset = num;

    //Evaluate content position
    bytes[0] = node->root->data->at(indexOffset, attributePosition + 20, lowerBound, upperBound);
    bytes[1] = node->root->data->at(indexOffset, attributePosition + 21, lowerBound, upperBound);
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    num = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
    uint64_t content_position = num; //@DM5

    attributePosition += num;

    //Store time since 1601/01/01
    //$STANDARD_INFORMATION Creation time
    if (offset - content_position > 0) { //@DM11
        bytes[0] = node->root->data->at(indexOffset, attributePosition, lowerBound, upperBound);
        bytes[1] = node->root->data->at(indexOffset, attributePosition + 1, lowerBound, upperBound);
        bytes[2] = node->root->data->at(indexOffset, attributePosition + 2, lowerBound, upperBound);
        bytes[3] = node->root->data->at(indexOffset, attributePosition + 3, lowerBound, upperBound);
        bytes[4] = node->root->data->at(indexOffset, attributePosition + 4, lowerBound, upperBound);
        bytes[5] = node->root->data->at(indexOffset, attributePosition + 5, lowerBound, upperBound);
        bytes[6] = node->root->data->at(indexOffset, attributePosition + 6, lowerBound, upperBound);
        bytes[7] = node->root->data->at(indexOffset, attributePosition + 7, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char *>(&num));

        if ((num > 0) && (num < 0x8000000000000000ULL)) {
            outputWriter->xmlout("CreationDate", nanoseconds2DateTime(num)); //@DM6 - if clause not necessary, STK has buggy timestamp conversion
         }
    }//@DM11

    //$STANDARD_INFORMATION Last modified time
    if (offset - content_position > 8) { //@DM11
        bytes[0] = node->root->data->at(indexOffset, attributePosition + 8, lowerBound, upperBound);
        bytes[1] = node->root->data->at(indexOffset, attributePosition + 9, lowerBound, upperBound);
        bytes[2] = node->root->data->at(indexOffset, attributePosition + 10, lowerBound, upperBound);
        bytes[3] = node->root->data->at(indexOffset, attributePosition + 11, lowerBound, upperBound);
        bytes[4] = node->root->data->at(indexOffset, attributePosition + 12, lowerBound, upperBound);
        bytes[5] = node->root->data->at(indexOffset, attributePosition + 13, lowerBound, upperBound);
        bytes[6] = node->root->data->at(indexOffset, attributePosition + 14, lowerBound, upperBound);
        bytes[7] = node->root->data->at(indexOffset, attributePosition + 15, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char *>(&num));

        if ((num > 0) && (num < 0x8000000000000000ULL)) {
            outputWriter->xmlout("LastModifiedDate", nanoseconds2DateTime(num)); //@DM6
        }
    } //@DM11

    //$STANDARD_INFORMATION Last modified time of MFT entry
    if (offset - content_position > 16) { //@DM11
        bytes[0] = node->root->data->at(indexOffset, attributePosition + 16, lowerBound, upperBound);
        bytes[1] = node->root->data->at(indexOffset, attributePosition + 17, lowerBound, upperBound);
        bytes[2] = node->root->data->at(indexOffset, attributePosition + 18, lowerBound, upperBound);
        bytes[3] = node->root->data->at(indexOffset, attributePosition + 19, lowerBound, upperBound);
        bytes[4] = node->root->data->at(indexOffset, attributePosition + 20, lowerBound, upperBound);
        bytes[5] = node->root->data->at(indexOffset, attributePosition + 21, lowerBound, upperBound);
        bytes[6] = node->root->data->at(indexOffset, attributePosition + 22, lowerBound, upperBound);
        bytes[7] = node->root->data->at(indexOffset, attributePosition + 23, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char *>(&num));

        if ((num > 0) && (num < 0x8000000000000000ULL)) {
            outputWriter->xmlout("LastModifiedDateMFT", nanoseconds2DateTime(num)); //@DM6
        }
    } //@DM11

    //$STANDARD_INFORMATION Last accessed time
    if (offset - content_position > 24) { //@DM11
        bytes[0] = node->root->data->at(indexOffset, attributePosition + 24, lowerBound, upperBound);
        bytes[1] = node->root->data->at(indexOffset, attributePosition + 25, lowerBound, upperBound);
        bytes[2] = node->root->data->at(indexOffset, attributePosition + 26, lowerBound, upperBound);
        bytes[3] = node->root->data->at(indexOffset, attributePosition + 27, lowerBound, upperBound);
        bytes[4] = node->root->data->at(indexOffset, attributePosition + 28, lowerBound, upperBound);
        bytes[5] = node->root->data->at(indexOffset, attributePosition + 29, lowerBound, upperBound);
        bytes[6] = node->root->data->at(indexOffset, attributePosition + 30, lowerBound, upperBound);
        bytes[7] = node->root->data->at(indexOffset, attributePosition + 31, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char *>(&num));

        if ((num > 0) && (num < 0x8000000000000000ULL)) {
            outputWriter->xmlout("LastAccessDate", nanoseconds2DateTime(num)); //@DM6
        }
    } //@DM11
}


void Ntfs_fl_dfxml::printFileName(AddressNodePtr node,dfxml_writer* outputWriter, uint64_t attributePosition, uint64_t indexOffset, uint64_t lowerBound, uint64_t upperBound) {
    //Variable initialization
    unsigned char bytes[256];
    uint64_t num;
    wchar_t unicode[100];

    //start @DM11
    //Evaluate attribute size and offset to next attribute
    bytes[0] = node->root->data->at(indexOffset, attributePosition + 4, lowerBound, upperBound);
    bytes[1] = node->root->data->at(indexOffset, attributePosition + 5, lowerBound, upperBound);
    bytes[2] = 0;
    bytes[3] = 0;
    num = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
    uint32_t offset = num;

    //Evaluate content position
    bytes[0] = node->root->data->at(indexOffset, attributePosition + 20, lowerBound, upperBound);
    bytes[1] = node->root->data->at(indexOffset, attributePosition + 21, lowerBound, upperBound);
    num = 0;
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
    uint64_t content_position = num;

    attributePosition += num;

    //Store $FILE_NAME name
    if (offset - content_position > 64) { //@DM11
        bytes[0] = node->root->data->at(indexOffset, attributePosition + 64, lowerBound, upperBound);
        uint8_t nameLength2 = bytes[0];
        nameLength2 = nameLength2 * 2;

        for (uint8_t i = 0; i < nameLength2; i++) {
            bytes[i] = node->root->data->at(indexOffset, attributePosition + 66 + i, lowerBound, upperBound);
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

        outputWriter->xmlout("Filename", nameString);
    }

    //$FILE_NAME actual size of file
    if (offset - content_position > 48) { //@DM11
        bytes[0] = node->root->data->at(indexOffset, attributePosition + 48, lowerBound, upperBound);
        bytes[1] = node->root->data->at(indexOffset, attributePosition + 49, lowerBound, upperBound);
        bytes[2] = node->root->data->at(indexOffset, attributePosition + 50, lowerBound, upperBound);
        bytes[3] = node->root->data->at(indexOffset, attributePosition + 51, lowerBound, upperBound);
        bytes[4] = node->root->data->at(indexOffset, attributePosition + 52, lowerBound, upperBound);
        bytes[5] = node->root->data->at(indexOffset, attributePosition + 53, lowerBound, upperBound);
        bytes[6] = node->root->data->at(indexOffset, attributePosition + 54, lowerBound, upperBound);
        bytes[7] = node->root->data->at(indexOffset, attributePosition + 55, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char *>(&num));

        outputWriter->xmlout("ActualSize", num);
    } //@DM11
}
