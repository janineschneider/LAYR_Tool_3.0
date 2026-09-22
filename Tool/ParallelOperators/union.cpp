#include "union.h"
#include <algorithm>
#include <iostream>

AddressNodeList Union::setOperator(AddressNodeList res1, AddressNodeList res2)
{
    AddressNodeList output;
    std::vector<bool> matched2(res2.size(), false);
    std::string m_tag = "+";

    // Process res1 and match against res2
    for (size_t idx1 = 0; idx1 < res1.size(); ++idx1) {
        auto& node1 = res1[idx1];

        if (!node1 || node1->m_data.empty()) {
            continue;
        }

        AddressNodePtr matched_node2 = nullptr;

        for (size_t idx2 = 0; idx2 < res2.size(); ++idx2) {
            auto& node2 = res2[idx2];
            if (!node2 || node2->m_data.empty()) {
                continue;
            }
            if (node1->m_data == node2->m_data) {
                matched_node2 = node2;
                matched2[idx2] = true;
                break;
            }
        }

        // If no match in res2, duplicate node
        if (!matched_node2) {
            AddressNodePtr new_node = std::make_shared<reconstructionNode>(node1->m_data, node1->m_metadata, m_tag);
            AddressNodeList new_child = { new_node };
            node1->add_children(new_child);
            output.push_back(new_node);
            continue;
        }

        // Merge metadata for matched nodes
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> merged_metadata;
        if (!node1->m_metadata.empty()) {
            for (const auto& seq : node1->m_metadata) {
                if (!seq.empty()) {
                    merged_metadata.push_back(seq);
                }
            }
        }
        if (!matched_node2->m_metadata.empty()) {
            for (const auto& seq : matched_node2->m_metadata) {
                if (!seq.empty()) {
                    merged_metadata.push_back(seq);
                }
            }
        }

        AddressNodePtr new_node = std::make_shared<reconstructionNode>(node1->m_data, merged_metadata, m_tag);
        output.push_back(new_node);

        AddressNodeList new_child = { new_node };
        node1->add_children(new_child);
        matched_node2->add_children(new_child);
    }

    // unmatched nodes in res2
    for (size_t i = 0; i < res2.size(); ++i) {
        if (!matched2[i] && res2[i] && !res2[i]->m_data.empty()) {
            AddressNodePtr new_node = std::make_shared<reconstructionNode>(res2[i]->m_data, res2[i]->m_metadata, m_tag);
            AddressNodeList new_child = { new_node };
            res2[i]->add_children(new_child);
            output.push_back(new_node);
        }
    }

    return output;
}