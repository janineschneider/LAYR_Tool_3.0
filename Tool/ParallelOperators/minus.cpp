#include "minus.h"
#include <algorithm>

AddressNodeList Minus::setOperator(AddressNodeList res1, AddressNodeList res2)
{
    AddressNodeList output;
    std::string tag = "-";

    for (auto& node1 : res1) {
        if (!node1 || node1->m_data.empty()) {
            continue;
        }

        bool match_found = false;
        AddressNodeList matched_node2s;

        // Check if node1 matches any node in res2
        for (auto& node2 : res2) {
            if (!node2 || node2->m_data.empty()) {
                continue;
            }

            if (node1->m_data == node2->m_data) {
                match_found = true;
                matched_node2s.push_back(node2);
            }
        }

        // Only add nodes from res1 that have NO matching content data in res2
        if (!match_found) {
            AddressNodePtr new_node = std::make_shared<reconstructionNode>(
                node1->m_data,
                node1->m_metadata,
                tag
            );

            // Establish child relationship
            AddressNodeList new_child = { new_node };
            node1->add_children(new_child);

            output.push_back(new_node);
        }
    }

    return output;
}