#include "fat32_fl_dfxml.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <bitset>
#include <cctype>

Fat32_fl_dfxml::Fat32_fl_dfxml() : m_rawData(nullptr)
{
}

Fat32_fl_dfxml::~Fat32_fl_dfxml()
{
}

AddressNodeList Fat32_fl_dfxml::evaluate(AddressNodeList input)
{

    try {
        //DFXML Handler
        outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
        outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

        for (AddressNodePtr node : input) {
            m_rawData = node->root->data;
            uint64_t id = 0;

            //Iterate inodes
            for (auto it = node->m_metadata.begin(); it != node->m_metadata.end(); ++it) {
                for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
                    lowerBound = it2->first;
                    upperBound = it2->second;
                    indexOffset = lowerBound;

                    //DFXML
                    outputWriter.push("fileobject");
                    outputWriter.xmlout("id", id);

                    Fat32_fl_dfxml::storeFileName();
                    Fat32_fl_dfxml::storeCreationTime();
                    Fat32_fl_dfxml::storeCreationDate();
                    Fat32_fl_dfxml::storeLastAccessedDate();
                    Fat32_fl_dfxml::storeLastWrittenTime();
                    Fat32_fl_dfxml::storeLastWrittenDate();
                    Fat32_fl_dfxml::storeFileSize();
                    Fat32_fl_dfxml::storeFirstCluster();

                    //Byte runs
                    outputWriter.push("byte_runs");
                    for (auto it3 = node->m_data.at(id).begin(); it3 != node->m_data.at(id).end(); ++it3) {
                        std::vector<std::string> tags = { "fs_offset", "len" };
                        std::vector<uint64_t> values = { it3->first, it3->second };
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
        }
        outputWriter.pop();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return input;
}


void Fat32_fl_dfxml::storeCreationTime()
{
    unsigned char bytes[3];
    bytes[0] = m_rawData->at(indexOffset + 13, lowerBound, upperBound);
    bytes[1] = m_rawData->at(indexOffset + 14, lowerBound, upperBound);
    bytes[2] = m_rawData->at(indexOffset + 15, lowerBound, upperBound);
    outputWriter.xmlout("creation_time", hex2Time(bytes, false));
}

void Fat32_fl_dfxml::storeCreationDate()
{
    unsigned char bytes[2];
    bytes[0] = m_rawData->at(indexOffset + 16, lowerBound, upperBound);
    bytes[1] = m_rawData->at(indexOffset + 17, lowerBound, upperBound);
    outputWriter.xmlout("creation_date", hex2Date(bytes));
}

void Fat32_fl_dfxml::storeLastAccessedDate()
{
    std::string lastAccessedTime = "00:00:00";
    unsigned char bytes[2];
    bytes[0] = m_rawData->at(indexOffset + 18, lowerBound, upperBound);
    bytes[1] = m_rawData->at(indexOffset + 19, lowerBound, upperBound);
    lastAccessedTime = hex2Date(bytes);
    outputWriter.xmlout("last_access_date", lastAccessedTime);
}


void Fat32_fl_dfxml::storeLastWrittenTime()
{
    unsigned char bytes[2];
    bytes[0] = m_rawData->at(indexOffset + 22, lowerBound, upperBound);
    bytes[1] = m_rawData->at(indexOffset + 23, lowerBound, upperBound);
    outputWriter.xmlout("last_written_time", hex2Time(bytes, false));
}

void Fat32_fl_dfxml::storeLastWrittenDate()
{
    unsigned char bytes[2];
    bytes[0] = m_rawData->at(indexOffset + 24, lowerBound, upperBound);
    bytes[1] = m_rawData->at(indexOffset + 25, lowerBound, upperBound);
    outputWriter.xmlout("last_written_date", hex2Date(bytes));
}

void Fat32_fl_dfxml::storeFileSize()
{
    unsigned char bytes[4];
    uint64_t num = 0;
    bytes[0] = m_rawData->at(indexOffset + 28, lowerBound, upperBound);
    bytes[1] = m_rawData->at(indexOffset + 29, lowerBound, upperBound);
    bytes[2] = m_rawData->at(indexOffset + 30, lowerBound, upperBound);
    bytes[3] = m_rawData->at(indexOffset + 31, lowerBound, upperBound);
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
    outputWriter.xmlout("filesize", num);
}

void Fat32_fl_dfxml::storeFirstCluster()
{
    unsigned char bytes[4];
    uint64_t num = 0;
    bytes[3] = m_rawData->at(indexOffset + 21, lowerBound, upperBound);
    bytes[2] = m_rawData->at(indexOffset + 20, lowerBound, upperBound);
    bytes[1] = m_rawData->at(indexOffset + 27, lowerBound, upperBound);
    bytes[0] = m_rawData->at(indexOffset + 26, lowerBound, upperBound);
    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
    outputWriter.xmlout("start_cluster", num);
}

void Fat32_fl_dfxml::storeFileName()
{
    bool lfn = false;
    bool deleted = false;
    uint32_t numberOfLFNEntries = 0;
    wchar_t unicode[13];
    unsigned char bytes[26];

    std::string shortFileName;
    std::string longFileName;

    //Check for long file name entry
    if (m_rawData->at(indexOffset + 11, lowerBound, upperBound) == 0x0f) {
        lfn = true;
    }

    //Set deleted true or false
    if (m_rawData->at(indexOffset, lowerBound, upperBound) == 0xe5) {
        deleted = true;
    }


    if (lfn) {
        //Get lenght of complete lfn entry
        if (!deleted) {
            numberOfLFNEntries = m_rawData->at(indexOffset, lowerBound, upperBound) & 0xf;
        }
        else {
            //In case of deleted number of lfn entries has to be calculated by number of
            //lfn markers because sequnece number is used to show allocation status
            numberOfLFNEntries = 1;
            uint64_t indexOffset2 = indexOffset;

            while (true) {
                if (m_rawData->at(indexOffset2 + 11 + 32, lowerBound, upperBound) == 0x0f) {
                    numberOfLFNEntries++;
                    indexOffset2 += 32;
                }
                else {
                    break;
                }
            }
        }

        while (numberOfLFNEntries > 0) {
            //Store long file name byte
            bytes[0] = m_rawData->at(indexOffset + 1, lowerBound, upperBound);
            bytes[1] = m_rawData->at(indexOffset + 2, lowerBound, upperBound);
            bytes[2] = m_rawData->at(indexOffset + 3, lowerBound, upperBound);
            bytes[3] = m_rawData->at(indexOffset + 4, lowerBound, upperBound);
            bytes[4] = m_rawData->at(indexOffset + 5, lowerBound, upperBound);
            bytes[5] = m_rawData->at(indexOffset + 6, lowerBound, upperBound);
            bytes[6] = m_rawData->at(indexOffset + 7, lowerBound, upperBound);
            bytes[7] = m_rawData->at(indexOffset + 8, lowerBound, upperBound);
            bytes[8] = m_rawData->at(indexOffset + 9, lowerBound, upperBound);
            bytes[9] = m_rawData->at(indexOffset + 10, lowerBound, upperBound);

            bytes[10] = m_rawData->at(indexOffset + 14, lowerBound, upperBound);
            bytes[11] = m_rawData->at(indexOffset + 15, lowerBound, upperBound);
            bytes[12] = m_rawData->at(indexOffset + 16, lowerBound, upperBound);
            bytes[13] = m_rawData->at(indexOffset + 17, lowerBound, upperBound);
            bytes[14] = m_rawData->at(indexOffset + 18, lowerBound, upperBound);
            bytes[15] = m_rawData->at(indexOffset + 19, lowerBound, upperBound);
            bytes[16] = m_rawData->at(indexOffset + 20, lowerBound, upperBound);
            bytes[17] = m_rawData->at(indexOffset + 21, lowerBound, upperBound);
            bytes[18] = m_rawData->at(indexOffset + 22, lowerBound, upperBound);
            bytes[19] = m_rawData->at(indexOffset + 23, lowerBound, upperBound);
            bytes[20] = m_rawData->at(indexOffset + 24, lowerBound, upperBound);
            bytes[21] = m_rawData->at(indexOffset + 25, lowerBound, upperBound);

            bytes[22] = m_rawData->at(indexOffset + 28, lowerBound, upperBound);
            bytes[23] = m_rawData->at(indexOffset + 29, lowerBound, upperBound);
            bytes[24] = m_rawData->at(indexOffset + 30, lowerBound, upperBound);
            bytes[25] = m_rawData->at(indexOffset + 31, lowerBound, upperBound);

            //Store long file name byte
            unicode[0] = bytes[1] << 8;
            unicode[0] = unicode[0] | bytes[0];
            unicode[1] = bytes[3] << 8;
            unicode[1] = unicode[1] | bytes[2];
            unicode[2] = bytes[5] << 8;
            unicode[2] = unicode[2] | bytes[4];
            unicode[3] = bytes[7] << 8;
            unicode[3] = unicode[3] | bytes[6];
            unicode[4] = bytes[9] << 8;
            unicode[4] = unicode[4] | bytes[8];

            unicode[5] = bytes[11] << 8;
            unicode[5] = unicode[5] | bytes[10];
            unicode[6] = bytes[13] << 8;
            unicode[6] = unicode[6] | bytes[12];
            unicode[7] = bytes[15] << 8;
            unicode[7] = unicode[7] | bytes[14];
            unicode[8] = bytes[17] << 8;
            unicode[8] = unicode[8] | bytes[16];
            unicode[9] = bytes[19] << 8;
            unicode[9] = unicode[9] | bytes[18];
            unicode[10] = bytes[21] << 8;
            unicode[10] = unicode[10] | bytes[20];

            unicode[11] = bytes[23] << 8;
            unicode[11] = unicode[11] | bytes[22];
            unicode[12] = bytes[25] << 8;
            unicode[12] = unicode[12] | bytes[24];

            std::wstring unicodeLFNString(unicode);
            std::string lfnString(unicodeLFNString.begin(), unicodeLFNString.end());
            if (lfnString.size() > 13) {
                lfnString.erase(13);
            }
            longFileName.insert(0, lfnString);

            numberOfLFNEntries--;
            indexOffset += 32;
        }
    }

    //Store short file name
    //Handle missing first character in deleted file case
    if (deleted) {
        bytes[0] = '_';
    }
    else {
        bytes[0] = m_rawData->at(indexOffset, lowerBound, upperBound);;
    }
    bytes[1] = m_rawData->at(indexOffset + 1, lowerBound, upperBound);
    bytes[2] = m_rawData->at(indexOffset + 2, lowerBound, upperBound);
    bytes[3] = m_rawData->at(indexOffset + 3, lowerBound, upperBound);
    bytes[4] = m_rawData->at(indexOffset + 4, lowerBound, upperBound);
    bytes[5] = m_rawData->at(indexOffset + 5, lowerBound, upperBound);
    bytes[6] = m_rawData->at(indexOffset + 6, lowerBound, upperBound);
    bytes[7] = m_rawData->at(indexOffset + 7, lowerBound, upperBound);
    bytes[8] = m_rawData->at(indexOffset + 8, lowerBound, upperBound);
    bytes[9] = m_rawData->at(indexOffset + 9, lowerBound, upperBound);
    bytes[10] = m_rawData->at(indexOffset + 10, lowerBound, upperBound);

    std::string helper(reinterpret_cast<char*>(bytes));
    if (helper.size() > 11) {
        helper.erase(11);
    }

    auto f = [](unsigned char const c) { return std::isspace(c); };

    //Short name handling for lfn and sfn
    if (lfn) {
        helper.erase(std::remove_if(helper.begin(), helper.end(), f), helper.end());

        //Handle file extension
        size_t found = helper.find("~");
        if ((found + 2) < helper.size()) {
            std::string prefix = "";
            prefix = helper.substr(found + 2, helper.size() - found);
            helper.erase(found + 2, helper.size() - found);
            helper.append(".");
            helper.append(prefix);
        }
        shortFileName = helper;
    }
    else {
        size_t found = helper.rfind(" ");
        helper.insert(found, ".");

        helper.erase(std::remove_if(helper.begin(), helper.end(), f), helper.end());
        shortFileName = helper;
        longFileName = helper;
    }
    outputWriter.xmlout("short_file_name", shortFileName);
    outputWriter.xmlout("long_file_name", longFileName);
}

std::string Fat32_fl_dfxml::hex2Date(unsigned char hex[])
{
    std::string dateString = "";
    uint16_t date = hex[0] | (hex[1] << 8);

    uint16_t lower_piece = date & 0x001f;

    uint16_t middle_piece = date & 0x01e0;
    middle_piece = middle_piece >> 5;

    uint16_t upper_piece = date & 0xfe00;
    upper_piece = upper_piece >> 9;

    dateString.append(std::to_string(upper_piece + 1980));
    dateString.append("-");
    if (middle_piece < 10) {
        dateString.append("0");
    }
    dateString.append(std::to_string(middle_piece));
    dateString.append("-");
    if (lower_piece < 10) {
        dateString.append("0");
    }
    dateString.append(std::to_string(lower_piece));

    return dateString;
}

std::string Fat32_fl_dfxml::hex2Time(unsigned char hex[], bool tenth)
{
    std::string timeString = "";
    uint16_t time = hex[1] | (hex[2] << 8);

    uint16_t lower_piece = time & 0x1f;

    uint16_t middle_piece = time & 0x7e0;
    middle_piece = middle_piece >> 5;

    uint16_t upper_piece = time & 0xf800;
    upper_piece = upper_piece >> 11;

    if (upper_piece < 10) {
        timeString.append("0");
    }
    timeString.append(std::to_string(upper_piece));
    timeString.append(":");
    if (middle_piece < 10) {
        timeString.append("0");
    }
    timeString.append(std::to_string(middle_piece));
    timeString.append(":");
    if (lower_piece < 10) {
        timeString.append("0");
    }
    timeString.append(std::to_string(lower_piece * 2));

    if (tenth) {
        timeString.pop_back();
        timeString.append(std::to_string(hex[0]));
        while (timeString.size() > 8) {
            timeString.pop_back();
        }
    }

    return timeString;
}