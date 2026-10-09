#include "ext4_fl_dfxml.h"
#include "../DFXML/dfxml_writer.h"

#include <ctime>
#include <iostream>
#include <map>
#include <sstream>

Ext4_fl_dfxml::Ext4_fl_dfxml() {}

Ext4_fl_dfxml::~Ext4_fl_dfxml() {}

void Ext4_fl_dfxml::evaluate(AddressNodeList input)
{
    try {
        // DFXML Handler
        dfxml_writer outputWriter;
        outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
        outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

        uint64_t id = 0;

        for (AddressNodePtr& node : input) {
            if (!node || !node->root || !node->root->data)
                continue;

            // Iterate inodes (one metadata sequence per file)
            for (size_t idx = 0; idx < node->m_metadata.size(); ++idx) {
                if (node->m_metadata.at(idx).empty())
                    continue;

                uint64_t lowerBound = node->m_metadata.at(idx).front().first;
                uint64_t upperBound = node->m_metadata.at(idx).front().second;
                uint64_t indexOffset = lowerBound;
                uint64_t inodeLength = upperBound - lowerBound + 1;

                try {
                    // Read everything first, so a failing read leaves the XML well-formed
                    // Store file size (lower 32 bit at 0x4, upper 32 bit at 0x6C)
                    uint64_t fileSize = readLE(node, indexOffset, 4, 4, lowerBound, upperBound) |
                        (readLE(node, indexOffset, 108, 4, lowerBound, upperBound) << 32);

                    // Links
                    uint64_t nlink = readLE(node, indexOffset, 26, 2, lowerBound, upperBound);

                    // Store uid (lower 16 bit at 0x2, upper 16 bit at 0x78)
                    uint64_t uid = readLE(node, indexOffset, 2, 2, lowerBound, upperBound) |
                        (readLE(node, indexOffset, 120, 2, lowerBound, upperBound) << 16);

                    // Store gid (lower 16 bit at 0x18, upper 16 bit at 0x7A)
                    uint64_t gid = readLE(node, indexOffset, 24, 2, lowerBound, upperBound) |
                        (readLE(node, indexOffset, 122, 2, lowerBound, upperBound) << 16);

                    // Timestamps
                    uint32_t mtime = (uint32_t)readLE(node, indexOffset, 16, 4, lowerBound, upperBound);
                    uint32_t ctime = (uint32_t)readLE(node, indexOffset, 12, 4, lowerBound, upperBound);
                    uint32_t atime = (uint32_t)readLE(node, indexOffset, 8, 4, lowerBound, upperBound);
                    uint32_t dtime = (uint32_t)readLE(node, indexOffset, 20, 4, lowerBound, upperBound);

                    // Creation time (only in extended inode, i_extra_isize at 0x80, i_crtime at 0x90)
                    bool hasCrtime = false;
                    uint32_t crtime = 0;
                    if (inodeLength >= 0x94) {
                        uint64_t extraIsize = readLE(node, indexOffset, 0x80, 2, lowerBound, upperBound);
                        if (extraIsize >= 20) {
                            crtime = (uint32_t)readLE(node, indexOffset, 0x90, 4, lowerBound, upperBound);
                            hasCrtime = true;
                        }
                    }

                    // DFXML
                    outputWriter.push("fileobject");

                    outputWriter.xmlout("id", id);
                    outputWriter.xmlout("filesize", fileSize);

                    // Allocation status
                    if (nlink > 0 && dtime == 0) {
                        outputWriter.xmlout("alloc", 1);
                    }
                    else {
                        outputWriter.xmlout("alloc", 0);
                    }

                    // TODO inode number
                    // outputWriter.xmlout("inode", "TODO");
                    // TODO File name
                    // outputWriter.xmlout("name", "TODO");

                    outputWriter.xmlout("nlink", nlink);
                    outputWriter.xmlout("uid", uid);
                    outputWriter.xmlout("gid", gid);

                    // File last modified date and time
                    outputWriter.xmlout("mtime", unix2DateTime(mtime));
                    // Inode last modified date and time
                    outputWriter.xmlout("ctime", unix2DateTime(ctime));
                    // Last accessed date and time
                    outputWriter.xmlout("atime", unix2DateTime(atime));
                    // Creation date and time
                    if (hasCrtime) {
                        outputWriter.xmlout("crtime", unix2DateTime(crtime));
                    }
                    // Deletion date and time
                    if (dtime != 0) {
                        outputWriter.xmlout("dtime", unix2DateTime(dtime));
                    }

                    // Byte runs
                    outputWriter.push("byte_runs");

                    if (idx < node->m_data.size()) {
                        for (auto it2 = node->m_data.at(idx).begin(); it2 != node->m_data.at(idx).end(); ++it2) {
                            std::vector<std::string> tags = { "img_offset", "len" };
                            std::vector<uint64_t> values = { it2->first, it2->second - it2->first + 1 };
                            std::string attributes = "";
                            for (size_t i = 0; i < tags.size(); i++) {
                                attributes.append(tags.at(i) + "=\"" + std::to_string(values.at(i)) + "\" ");
                            }

                            outputWriter.xmlout("byte_run", "", attributes, true);
                        }
                    }

                    outputWriter.pop();
                    outputWriter.pop();
                    id++;
                }
                catch (std::exception& e) {
                    std::cout << e.what() << std::endl;
                }
            }
        }
        outputWriter.pop();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

std::string Ext4_fl_dfxml::unix2DateTime(uint32_t u)
{
    time_t t = (time_t)u;
    struct tm* tm = localtime(&t);
    char date[20];
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", tm);

    std::string dateTime(date);

    return dateTime + " (CET)";
}

uint64_t Ext4_fl_dfxml::readLE(AddressNodePtr& node, uint64_t offset, uint64_t position, int bytes,
    uint64_t lowerBound, uint64_t upperBound)
{
    uint64_t value = 0;
    for (int i = 0; i < bytes; i++) {
        value |= (uint64_t)node->root->data->at(offset, position + i, lowerBound, upperBound) << (8 * i);
    }
    return value;
}