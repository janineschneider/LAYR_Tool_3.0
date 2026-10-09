#include "ext4_fsl_dfxml.h"
#include "../DFXML/dfxml_writer.h"

#include <iostream>
#include <map>
#include <sstream>
#include <fstream>

Ext4_fsl_dfxml::Ext4_fsl_dfxml() {}

Ext4_fsl_dfxml::~Ext4_fsl_dfxml() {}

void Ext4_fsl_dfxml::evaluate(AddressNodeList input)
{
    try {
        // DFXML Handler
        dfxml_writer outputWriter;
        outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
        outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

        for (AddressNodePtr& node : input) {
            if (!node || !node->root || !node->root->data)
                continue;
            if (node->m_data.empty() || node->m_data.front().empty() ||
                node->m_metadata.empty() || node->m_metadata.front().empty())
                continue;

            // File system range and superblock (created by Ext4_fsl)
            uint64_t indexOffset = node->m_data.front().front().first;
            uint64_t sbStart = node->m_metadata.front().front().first;
            uint64_t sbEnd = node->m_metadata.front().front().second;

            try {
                // Store block size in byte
                uint64_t blockSize = 1024ULL << readLE(node, sbStart, 0x18, 4, sbStart, sbEnd);

                // Store numbers of blocks in file system
                uint64_t blockNumber = readLE(node, sbStart, 0x4, 4, sbStart, sbEnd);
                uint64_t fileSystemSize = blockNumber * blockSize;

                // Store inode count and inode record size
                uint64_t inodeCount = readLE(node, sbStart, 0x0, 4, sbStart, sbEnd);
                uint64_t inodeSize = readLE(node, sbStart, 0x58, 2, sbStart, sbEnd);

                // Create DFXML output
                outputWriter.push("volume");
                outputWriter.push("byte_runs");

                std::vector<std::string> tags = { "img_offset", "len" };
                std::vector<uint64_t> values = { indexOffset, fileSystemSize };
                std::string attributes = "";
                for (size_t i = 0; i < tags.size(); i++) {
                    attributes.append(tags.at(i) + "=\"" + std::to_string(values.at(i)) + "\" ");
                }

                outputWriter.xmlout("byte_run", "", attributes, true);
                outputWriter.pop();

                outputWriter.xmlout("block_size", blockSize);
                outputWriter.xmlout("ftype_str", "Ext4");
                outputWriter.xmlout("block_count", blockNumber);
                outputWriter.xmlout("first_block", (uint64_t)0);
                outputWriter.xmlout("last_block", (blockNumber > 0) ? blockNumber - 1 : 0);
                outputWriter.xmlout("inode_count", inodeCount);
                outputWriter.xmlout("inode_size", inodeSize);

                outputWriter.pop();
            }
            catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }
        outputWriter.pop();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

uint64_t Ext4_fsl_dfxml::readLE(AddressNodePtr& node, uint64_t offset, uint64_t position, int bytes,
    uint64_t lowerBound, uint64_t upperBound)
{
    uint64_t value = 0;
    for (int i = 0; i < bytes; i++) {
        value |= (uint64_t)node->root->data->at(offset, position + i, lowerBound, upperBound) << (8 * i);
    }
    return value;
}