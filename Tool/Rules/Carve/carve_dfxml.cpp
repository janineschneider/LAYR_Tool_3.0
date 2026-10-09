#include "carve_dfxml.h"
#include "../DFXML/dfxml_writer.h"

#include <iostream>
#include <map>
#include <sstream>

Carve_dfxml::Carve_dfxml() {}

Carve_dfxml::~Carve_dfxml() {}

void Carve_dfxml::evaluate(AddressNodeList input)
{
    try {
        // DFXML Handler
        dfxml_writer outputWriter;
        outputWriter.push("dfxml", "xmloutputversion=\"1.2.0\"");
        outputWriter.add_DFXML_creator("LAYR", "2.1", "2.1", "");

        uint64_t id = 0;
        const std::string prefix = "carve_";

        for (AddressNodePtr& node : input) {
            if (!node)
                continue;

            // File type is stored in the tag ("carve_jpg" -> "jpg")
            std::string type = node->m_tag;
            if (type.compare(0, prefix.size(), prefix) == 0) {
                type = type.substr(prefix.size());
            }

            // File size
            uint64_t fileSize = 0;
            for (auto it = node->m_data.begin(); it != node->m_data.end(); ++it) {
                for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
                    fileSize += it2->second - it2->first + 1;
                }
            }

            outputWriter.push("fileobject");

            outputWriter.xmlout("id", id);
            outputWriter.xmlout("filetype", type);
            outputWriter.xmlout("filesize", fileSize);

            // Byte runs
            outputWriter.push("byte_runs");

            for (auto it = node->m_data.begin(); it != node->m_data.end(); ++it) {
                for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
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
        outputWriter.pop();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}