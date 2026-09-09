#include "ext3_fsl_dfxml.h"
#include "../DFXML/dfxml_writer.h"

#include <iostream>
#include <map>
#include <sstream>
#include <fstream>

Ext3_fsl_dfxml::Ext3_fsl_dfxml() {}

Ext3_fsl_dfxml::~Ext3_fsl_dfxml() {}

void Ext3_fsl_dfxml::evaluate(AddressNodeList input)
{
    // Helper
    // LowerBound also functions as index offset to lowest input level
    // TODO Use more than first object
    // TODO Handle fragmentation
    // TODO Use metadata instead of contentdata

    // Helper
    unsigned char bytes[4];
    uint32_t num32 = 0;

    // Essential Ext3 file system information
    uint32_t blockSize = 0;
    uint32_t blockNumber = 0;
    uint64_t fileSystemSize = 0;

    // DFXML Handler
    dfxml_writer outputWriter;
    outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
    outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

    // Create DFXML output
    outputWriter.push("volume");
    outputWriter.push("byte_runs");

    for (AddressNodePtr &node : input)
    {
        if (node->m_data.size() > 0)
        {

            uint64_t lowerBound = node->m_data.front().front().first;
            uint64_t upperBound = node->m_data.back().back().second;
            uint64_t indexOffset = node->m_data.front().front().first;

            // Store block size in byte
            bytes[0] = node->root->data->at(indexOffset + 1024, 24, lowerBound, upperBound);
            bytes[1] = node->root->data->at(indexOffset + 1024, 25, lowerBound, upperBound);
            bytes[2] = node->root->data->at(indexOffset + 1024, 26, lowerBound, upperBound);
            bytes[3] = node->root->data->at(indexOffset + 1024, 27, lowerBound, upperBound);
            num32 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num32));
            blockSize = 1024 << num32;

            // Store numbers of blocks in file system
            bytes[0] = node->root->data->at(indexOffset + 1024, 4, lowerBound, upperBound);
            bytes[1] = node->root->data->at(indexOffset + 1024, 5, lowerBound, upperBound);
            bytes[2] = node->root->data->at(indexOffset + 1024, 6, lowerBound, upperBound);
            bytes[3] = node->root->data->at(indexOffset + 1024, 7, lowerBound, upperBound);
            num32 = 0;
            std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char *>(&num32));
            blockNumber = num32;
            fileSystemSize = blockNumber * blockSize;

            std::vector<std::string> tags = {"fs_offset", "len"};
            std::vector<uint64_t> values = {indexOffset, fileSystemSize};
            std::string attributes = "";
            for (size_t i = 0; i < tags.size(); i++)
            {
                attributes.append(tags.at(i) + "=\"" + std::to_string(values.at(i)) + "\" ");
            }

            outputWriter.xmlout("byte_run", "", attributes, true);
            outputWriter.pop();

            outputWriter.xmlout("block_size", blockSize);
            outputWriter.xmlout("ftype_str", "Ext3");
            outputWriter.xmlout("block_count", blockNumber);
            outputWriter.xmlout("first_block", indexOffset);
            outputWriter.xmlout("last_block", (upperBound - blockSize));
        }
        else
        {

            outputWriter.pop();
        }

        outputWriter.pop();
        outputWriter.pop();
    }
}
