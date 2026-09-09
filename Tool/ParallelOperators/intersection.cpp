#include "intersection.h"
#include <algorithm>

AddressNodeList Intersection::setOperator(AddressNodeList res1, AddressNodeList res2)
{
    AddressNodeList output;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> intersected_data;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> intersected_metadata;
    std::string m_tag = "·";
    AddressNodePtr new_node;
    AddressNodeList new_child;

    for (auto& node1 : res1) {
        if (node1->m_data.empty()) {
            continue;
        }

        for (auto& node2 : res2) {
            if (node2->m_data.empty()) {
                continue;
            }

            if (node1->m_data == node2->m_data) {

                intersected_data = node1->m_data;
                intersected_metadata = node1->m_metadata;

                for (const auto& m2 : node2->m_metadata) {
                    for (const auto& pair : m2) {

                        bool exists = false;
                        for (const auto& i_m : intersected_metadata) {
                            if (std::find(i_m.begin(), i_m.end(), pair) != i_m.end()) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            intersected_metadata.push_back({ pair });
                        }
                    }
                }

                AddressNodePtr new_node = std::make_shared<reconstructionNode>(intersected_data, intersected_metadata, m_tag);

                AddressNodeList new_child = { new_node };
                node1->add_children(new_child);
                node2->add_children(new_child);
                output.push_back(new_node);
            }
        }
    }
    return output;
}
