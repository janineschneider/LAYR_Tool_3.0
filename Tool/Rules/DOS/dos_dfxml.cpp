#include "dos_dfxml.h"
#include "../DFXML/dfxml_writer.h"

#include <iostream>
#include <map>
#include <sstream>

static const std::map<unsigned char, std::string> partitionTypes = {
    { 0x00, "Empty (0x00)" },
    { 0x01, "FAT12, CHS (0x01)" },
    { 0x04, "DOS FAT16 (0x04)" },
    { 0x05, "DOS Extended (0x05)" },
    { 0x06, "DOS FAT16 (0x06)" },
    { 0x07, "NTFS (0x07)" },
    { 0x0b, "FAT32, CHS (0x0b)" },
    { 0x0c, "FAT32, LBA (0x0c)" },
    { 0x0e, "FAT16, 32 MB-2GB, LBA (0x0e)" },
    { 0x0f, "DOS Extended (0x0f)" },
    { 0x11, "Hidden FAT12, CHS (0x11)" },
    { 0x14, "Hidden FAT 16, 16-32 MB, CHS (0x14)" },
    { 0x16, "Hidden FAT16, 32 MB-2GB, CHS (0x16)" },
    { 0x1b, "Hidden FAT32, CHS (0x1b)" },
    { 0x1c, "Hidden FAT32, LBA (0x1c)" },
    { 0x1e, "Hidden FAT16, 32 MB-2GB, LBA (0x1e)" },
    { 0x42, "Microsoft MBR, Dynamic Disk (0x42)" },
    { 0x82, "Solaris x86, Linux Swap (0x82)" },
    { 0x83, "Linux (0x83)" },
    { 0x84, "Hibernation (0x84)" },
    { 0x85, "Linux Extended (0x85)" },
    { 0x86, "NTFS Volume Set (0x86)" },
    { 0x87, "NTFS Volume Set (0x87)" },
    { 0xa0, "Hibernation (0xa0)" },
    { 0xa1, "Hibernation (0xa1)" },
    { 0xa5, "FreeBSD (0xa5)" },
    { 0xa6, "OpenBSD (0xa6)" },
    { 0xa8, "Mac OSX (0xa8)" },
    { 0xa9, "NetBSD (0xa9)" },
    { 0xab, "Mac OSX Boot (0xab)" },
    { 0xb7, "BSDI (0xb7)" },
    { 0xb8, "BSDI swap (0xb8)" },
    { 0xee, "EFI GPT Disk (0xee)" },
    { 0xef, "EFI System Partition (0xef)" },
    { 0xfb, "Vmware File System (0xfb)" },
    { 0xfc, "Vmware swap (0xfc)" },
};

DOS_dfxml::DOS_dfxml() {}

DOS_dfxml::~DOS_dfxml() {}

void DOS_dfxml::evaluate(AddressNodeList input)
{
    try {
        // DFXML Handler
        dfxml_writer outputWriter;
        outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
        outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

        for (AddressNodePtr& node : input) {
            if (!node || !node->root || !node->root->data)
                continue;

            // Each metadata sequence holds one partition table entry
            for (auto it = node->m_metadata.begin(); it != node->m_metadata.end(); ++it) {
                if (it->empty())
                    continue;

                // TODO Handle fragmentation?
                uint64_t entryStart = it->at(0).first;
                uint64_t lowerBound = it->at(0).first;
                uint64_t upperBound = it->at(0).second;

                try {
                    // Read everything first, so a failing read leaves the XML well-formed
                    unsigned char typeByte = (unsigned char)readLE(node, entryStart, 4, 1, lowerBound, upperBound);

                    // WARNING Starting address of partitions within extended partitions are given without offset!
                    // WARNING Pure extended partition table entry information will be printed!
                    uint64_t partitionStart = readLE(node, entryStart, 8, 4, lowerBound, upperBound) * 512;
                    uint64_t partitionSize = readLE(node, entryStart, 12, 4, lowerBound, upperBound) * 512;

                    auto type = partitionTypes.find(typeByte);
                    std::string typeStr = (type != partitionTypes.end()) ? type->second : "Unknown";

                    // DFXML
                    outputWriter.push("partitionobject");
                    outputWriter.xmlout("ptype", (uint64_t)typeByte);
                    outputWriter.xmlout("ptype_str", typeStr);

                    outputWriter.push("byte_runs");
                    std::vector<std::string> tags = { "img_offset", "len" };
                    std::vector<uint64_t> values = { partitionStart, partitionSize };
                    std::string attributes = "";
                    for (size_t i = 0; i < tags.size(); i++) {
                        attributes.append(tags.at(i) + "=\"" + std::to_string(values.at(i)) + "\" ");
                    }
                    outputWriter.xmlout("byte_run", "", attributes, true);
                    outputWriter.pop();

                    outputWriter.pop();
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

uint64_t DOS_dfxml::readLE(AddressNodePtr& node, uint64_t offset, uint64_t position, int bytes,
    uint64_t lowerBound, uint64_t upperBound)
{
    uint64_t value = 0;
    for (int i = 0; i < bytes; i++) {
        value |= (uint64_t)node->root->data->at(offset, position + i, lowerBound, upperBound) << (8 * i);
    }
    return value;
}