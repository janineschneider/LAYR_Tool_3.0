#include "fat32_fsl_dfxml.h"
#include "../DFXML/dfxml_writer.h"

#include <iostream>
#include <map>
#include <sstream>

Fat32_fsl_dfxml::Fat32_fsl_dfxml()
{
}

Fat32_fsl_dfxml::~Fat32_fsl_dfxml()
{
}

AddressNodeList Fat32_fsl_dfxml::evaluate(AddressNodeList input)
{
    for (AddressNodePtr node : input) {
        //Helper
        //LowerBound also functions as index offset to lowest input level
        uint64_t lowerBound = node->m_data.front().front().first;
        uint64_t upperBound = node->m_data.back().back().second;
        uint64_t indexOffset = node->m_data.front().front().first;

        //Helper
        unsigned char bytes[11];
        uint32_t num = 0;

        //Essential Ext3 file system information
        std::string fsLabel;

        //DFXML Handler
        dfxml_writer outputWriter;
        outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
        outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

        //Create DFXML output
        outputWriter.push("volume");
        outputWriter.push("byte_runs");

        //Extract size of reserved area (in sectors)
        bytes[0] = node->root->data->at(indexOffset + 14, lowerBound, upperBound);
        bytes[1] = node->root->data->at(indexOffset + 15, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
        outputWriter.xmlout("reserved_area_size", num);

        //Extract sector size (bytes per sector)
        bytes[0] = node->root->data->at(indexOffset + 11, lowerBound, upperBound);
        bytes[1] = node->root->data->at(indexOffset + 12, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char*>(&num));
        uint16_t sectorSize = num;
        outputWriter.xmlout("sector_size", sectorSize);

        //Store cluster size (sectors per cluster)
        num = node->root->data->at(indexOffset + 13, lowerBound, upperBound);
        //Convert to bytes per cluster
        outputWriter.xmlout("cluster_size", num * sectorSize);

        //Size of FAT
        bytes[0] = node->root->data->at(indexOffset + 36, lowerBound, upperBound);
        bytes[1] = node->root->data->at(indexOffset + 37, lowerBound, upperBound);
        bytes[2] = node->root->data->at(indexOffset + 38, lowerBound, upperBound);
        bytes[3] = node->root->data->at(indexOffset + 39, lowerBound, upperBound);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint32_t), reinterpret_cast<unsigned char*>(&num));
        outputWriter.xmlout("fat_size", num);

        outputWriter.pop();
        outputWriter.pop();
        outputWriter.pop();
    }

    return input;
}