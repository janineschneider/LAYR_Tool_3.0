#include "ext3_fl_dfxml.h"
#include "../DFXML/dfxml_writer.h"

#include <iostream>
#include <map>
#include <sstream>

Ext3_fl_dfxml::Ext3_fl_dfxml() {}

Ext3_fl_dfxml::~Ext3_fl_dfxml() {}

void Ext3_fl_dfxml::evaluate(AddressNodeList input)
{
    std::set<uint64_t> processed_ids;
    try
    {
        // DFXML Handler
        dfxml_writer outputWriter;
        outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
        outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

        for (AddressNodePtr &node : input)
        {
            // Helper
            uint64_t lowerBound = 0;
            uint64_t upperBound = 0;
            uint64_t indexOffset = 0;
            uint64_t id = 0;
            unsigned char bytes[4];
            uint64_t num = 0;
            uint32_t fileSize = 0;

            // Get ROCompatFeatures
            bytes[0] = node->root->data->at(1024, 100, 1024, 2047);
            std::string rocompatFeatures = evaluateROCompatFeatures(bytes);

            // Iterate inodes
            for (auto it = node->m_metadata.begin(); it != node->m_metadata.end(); ++it)
            {
                for (auto it2 = it->begin(); it2 != it->end(); ++it2)
                {
                    lowerBound = it2->first;
                    upperBound = it2->second;
                    indexOffset = lowerBound;

                    // DFXML
                    if (processed_ids.find(id) != processed_ids.end())
                    {
                        id++;
                        continue; // Jump if this id has already been written
                    }
                    processed_ids.insert(id);

                    outputWriter.push("fileobject");

                    outputWriter.xmlout("id", id);

                    // Store file size
                    // begin @DM14
                    if (rocompatFeatures.find("Large File") != std::string::npos)
                    {
                        unsigned char largefilesize[8];
                        largefilesize[0] = node->root->data->at(indexOffset + 4, lowerBound, upperBound);
                        largefilesize[1] = node->root->data->at(indexOffset + 5, lowerBound, upperBound);
                        largefilesize[2] = node->root->data->at(indexOffset + 6, lowerBound, upperBound);
                        largefilesize[3] = node->root->data->at(indexOffset + 7, lowerBound, upperBound);
                        largefilesize[4] = node->root->data->at(indexOffset + 108, lowerBound, upperBound);
                        largefilesize[5] = node->root->data->at(indexOffset + 109, lowerBound, upperBound);
                        largefilesize[6] = node->root->data->at(indexOffset + 110, lowerBound, upperBound);
                        largefilesize[7] = node->root->data->at(indexOffset + 111, lowerBound, upperBound);
                        std::copy(&(largefilesize[0]), &(largefilesize[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char *>(&num));
                    }
                    else
                    {
                        bytes[0] = node->root->data->at(indexOffset + 4, lowerBound, upperBound);
                        bytes[1] = node->root->data->at(indexOffset + 5, lowerBound, upperBound);
                        bytes[2] = node->root->data->at(indexOffset + 6, lowerBound, upperBound);
                        bytes[3] = node->root->data->at(indexOffset + 7, lowerBound, upperBound);
                        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num));
                    }
                    outputWriter.xmlout("filesize", num);
                    fileSize = num;
                    // end @DM14

                    // Allocation status
                    if (num > 0)
                    {
                        outputWriter.xmlout("alloc", 1);
                    }
                    else
                    {
                        outputWriter.xmlout("alloc", 0);
                    }

                    // TODO inode number
                    // outputWriter.xmlout("inode", "TODO");
                    // TODO File name
                    // outputWriter.xmlout("name", "TODO");

                    // Links
                    bytes[0] = node->root->data->at(indexOffset + 26, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(indexOffset + 27, lowerBound, upperBound);
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char *>(&num));
                    outputWriter.xmlout("nlink", num);

                    // begin @DM12
                    // Store uid (user identifier)
                    bytes[0] = node->root->data->at(indexOffset + 2, lowerBound, upperBound); // two lower bits
                    bytes[1] = node->root->data->at(indexOffset + 3, lowerBound, upperBound);
                    bytes[2] = node->root->data->at(indexOffset + 120, lowerBound, upperBound); // two upper bits
                    bytes[3] = node->root->data->at(indexOffset + 121, lowerBound, upperBound);
                    num = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num));
                    outputWriter.xmlout("uid", std::to_string(num));

                    // Store gid (group identifier)
                    bytes[0] = node->root->data->at(indexOffset + 24, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(indexOffset + 25, lowerBound, upperBound);
                    bytes[2] = node->root->data->at(indexOffset + 122, lowerBound, upperBound);
                    bytes[3] = node->root->data->at(indexOffset + 123, lowerBound, upperBound);
                    num = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num));
                    outputWriter.xmlout("gid", std::to_string(num));
                    // end @DM12

                    // File last modified date and time
                    bytes[0] = node->root->data->at(indexOffset + 16, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(indexOffset + 17, lowerBound, upperBound);
                    bytes[2] = node->root->data->at(indexOffset + 18, lowerBound, upperBound);
                    bytes[3] = node->root->data->at(indexOffset + 19, lowerBound, upperBound);
                    num = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num));
                    outputWriter.xmlout("mtime", unix2DateTime(num));

                    // Inode last modified date and time
                    bytes[0] = node->root->data->at(indexOffset + 12, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(indexOffset + 13, lowerBound, upperBound);
                    bytes[2] = node->root->data->at(indexOffset + 14, lowerBound, upperBound);
                    bytes[3] = node->root->data->at(indexOffset + 15, lowerBound, upperBound);
                    num = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num));
                    outputWriter.xmlout("ctime", unix2DateTime(num));

                    // Last accessed date and time
                    bytes[0] = node->root->data->at(indexOffset + 8, lowerBound, upperBound);
                    bytes[1] = node->root->data->at(indexOffset + 9, lowerBound, upperBound);
                    bytes[2] = node->root->data->at(indexOffset + 10, lowerBound, upperBound);
                    bytes[3] = node->root->data->at(indexOffset + 11, lowerBound, upperBound);
                    num = 0;
                    std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num));
                    outputWriter.xmlout("atime", unix2DateTime(num));

                    // Deletion date and time
                    if (fileSize <= 0)
                    {
                        bytes[0] = node->root->data->at(indexOffset + 20, lowerBound, upperBound);
                        bytes[1] = node->root->data->at(indexOffset + 21, lowerBound, upperBound);
                        bytes[2] = node->root->data->at(indexOffset + 22, lowerBound, upperBound);
                        bytes[3] = node->root->data->at(indexOffset + 23, lowerBound, upperBound);
                        num = 0;
                        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num));
                        outputWriter.xmlout("dtime", unix2DateTime(num));
                    }

                    // Byte runs
                    outputWriter.push("byte_runs");

                    for (auto it2 = node->m_data.at(id).begin(); it2 != node->m_data.at(id).end(); ++it2)
                    {
                        std::vector<std::string> tags = {"fs_offset", "len"};
                        std::vector<uint64_t> values = {it2->first, it2->second};
                        std::string attributes = "";
                        for (size_t i = 0; i < tags.size(); i++)
                        {
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
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
}

std::string Ext3_fl_dfxml::unix2DateTime(uint32_t u)
{
    time_t t = (time_t)u;
    struct tm *tm = localtime(&t);
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm);

    std::string dateTime(date);

    return dateTime + " (CET)";
}

std::string Ext3_fl_dfxml::evaluateROCompatFeatures(unsigned char hex[])
{
    uint8_t dec = 0;
    std::copy(&(hex[0]), &(hex[0]) + sizeof(uint8_t), reinterpret_cast<unsigned char *>(&dec));
    std::string helper = "";

    // Switch compat feature
    if (dec & 0x0001)
        helper.append("Sparse Super, ");
    if (dec & 0x0002)
        helper.append("Large File, ");
    if (dec & 0x0004)
        helper.append("Huge File, ");
    if (dec & 0x0008)
        helper.append("Btree Dir, ");
    if (dec & 0x0010)
        helper.append("Extra Inode Size");

    return helper;
}
