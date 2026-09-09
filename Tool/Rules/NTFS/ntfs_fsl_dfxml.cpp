#include "ntfs_fsl_dfxml.h"
#include "../DFXML/dfxml_writer.h"

#include <iostream>
#include <map>
#include <sstream>

Ntfs_fsl_dfxml::Ntfs_fsl_dfxml() {}

Ntfs_fsl_dfxml::~Ntfs_fsl_dfxml() {}

void Ntfs_fsl_dfxml::evaluate(AddressNodeList input) {
    unsigned char bytes[8];
    uint64_t num = 0;

    // DFXML Handler
    dfxml_writer outputWriter;
    outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
    outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

    outputWriter.push("boot");

    for (AddressNodePtr &node : input) {
        // Helper
        uint64_t indexOffset = node->m_data.front().front().first;
        uint64_t lowerBoundBootSector = node->m_metadata.front().front().first;
        uint64_t upperBoundBootSector = node->m_metadata.front().front().second;
        uint64_t lowerBoundMFT = node->m_metadata.back().front().first;
        uint64_t upperBoundMFT = node->m_metadata.back().front().second;

        std::vector<std::string> tags = {"start", "end"};
        std::vector<uint64_t> values = {lowerBoundBootSector, upperBoundBootSector};
        std::string attributes = "";
        for (size_t i = 0; i < tags.size(); i++) {
            attributes.append(tags.at(i) + "=\"" + std::to_string(values.at(i)) + "\" ");
        }
        outputWriter.xmlout("boot_sector", "", attributes, true);

        // Store bytes per sector
        bytes[0] = node->root->data->at(11, lowerBoundBootSector, upperBoundBootSector);
        bytes[1] = node->root->data->at(12, lowerBoundBootSector, upperBoundBootSector);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint16_t), reinterpret_cast<unsigned char *>(&num));
        uint16_t bytes_per_sector = num;
        outputWriter.xmlout("bytes_per_sector", bytes_per_sector);

        // Store sectors per cluster
        bytes[0] = node->root->data->at(13, lowerBoundBootSector, upperBoundBootSector);
        num = bytes[0];
        outputWriter.xmlout("sectors_per_cluster", num);

        // Store first cluster of MFT
        bytes[0] = node->root->data->at(48, lowerBoundBootSector, upperBoundBootSector);
        bytes[1] = node->root->data->at(49, lowerBoundBootSector, upperBoundBootSector);
        bytes[2] = node->root->data->at(50, lowerBoundBootSector, upperBoundBootSector);
        bytes[3] = node->root->data->at(51, lowerBoundBootSector, upperBoundBootSector);
        bytes[4] = node->root->data->at(52, lowerBoundBootSector, upperBoundBootSector);
        bytes[5] = node->root->data->at(53, lowerBoundBootSector, upperBoundBootSector);
        bytes[6] = node->root->data->at(54, lowerBoundBootSector, upperBoundBootSector);
        bytes[7] = node->root->data->at(55, lowerBoundBootSector, upperBoundBootSector);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char *>(&num));
        outputWriter.xmlout("first_cluster_of_MFT", lowerBoundMFT);

        outputWriter.pop();

        outputWriter.push("mft");
        tags = {"start", "end"};
        values = {lowerBoundMFT, upperBoundMFT};
        attributes = "";
        for (size_t i = 0; i < tags.size(); i++) {
            attributes.append(tags.at(i) + "=\"" + std::to_string(values.at(i)) + "\" ");
        }
        outputWriter.xmlout("mft_sector", "", attributes, true);
        outputWriter.pop();

        // Create DFXML output
        outputWriter.push("volume");
        outputWriter.push("byte_runs");

        // Store total number of sectors
        bytes[0] = node->root->data->at(40, lowerBoundBootSector, upperBoundBootSector);
        bytes[1] = node->root->data->at(41, lowerBoundBootSector, upperBoundBootSector);
        bytes[2] = node->root->data->at(42, lowerBoundBootSector, upperBoundBootSector);
        bytes[3] = node->root->data->at(43, lowerBoundBootSector, upperBoundBootSector);
        bytes[4] = node->root->data->at(44, lowerBoundBootSector, upperBoundBootSector);
        bytes[5] = node->root->data->at(45, lowerBoundBootSector, upperBoundBootSector);
        bytes[6] = node->root->data->at(46, lowerBoundBootSector, upperBoundBootSector);
        bytes[7] = node->root->data->at(47, lowerBoundBootSector, upperBoundBootSector);
        num = 0;
        std::copy(&(bytes[0]), &(bytes[0]) + sizeof(uint64_t), reinterpret_cast<unsigned char *>(&num));
        uint64_t totalSectorRange = num - 1;

        tags = {"fs_offset", "len"};
        values = {indexOffset, totalSectorRange * bytes_per_sector};
        attributes = "";
        for (size_t i = 0; i < tags.size(); i++) {
            attributes.append(tags.at(i) + "=\"" + std::to_string(values.at(i)) + "\" ");
        }

        outputWriter.xmlout("byte_run", "", attributes, true);
        outputWriter.pop();
        outputWriter.pop();
        outputWriter.pop();
    }
}
