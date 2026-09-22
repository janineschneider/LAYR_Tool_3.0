#include "gtest/gtest.h"
#include "tree_test_fixture.hpp"
#include "../tree.h"
#include "../ParallelOperators/union.h"
#include <iostream>
#include <algorithm>

TEST_F(TreeTestFixture, JoiningOverlappingPairs)
{
    AddLayer1();

    AddressNodeList res1 = { tree->tree->m_children->at(0) };

    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> partition0 = { { { 0, 1 } } };
    AddressNodePtr matching_node = MakeReconstructionNode("Partition_Ext4", partition0);

    tree->tree->add_children({ matching_node });

    AddressNodeList res2 = { matching_node };

    Union union_op(nullptr, nullptr);
    AddressNodeList output = union_op.setOperator(res1, res2);

    ASSERT_FALSE(output.empty());
    AssertTreeIntegrity();
}

TEST_F(TreeTestFixture, JoiningOverlappingPairsManualNodes)
{
    std::pair<uint64_t, uint64_t> c1 = { 10, 20 };
    std::pair<uint64_t, uint64_t> c2 = { 30, 40 };
    std::pair<uint64_t, uint64_t> c3 = { 50, 60 };
    std::pair<uint64_t, uint64_t> c4 = { 70, 80 };
    std::pair<uint64_t, uint64_t> c5 = { 90, 100 };

    std::pair<uint64_t, uint64_t> m1_1 = { 110, 120 };
    std::pair<uint64_t, uint64_t> m1_2 = { 130, 140 };
    std::pair<uint64_t, uint64_t> m2_1 = { 150, 160 };
    std::pair<uint64_t, uint64_t> m2_2 = { 170, 180 };
    std::pair<uint64_t, uint64_t> m3_1 = { 190, 200 };
    std::pair<uint64_t, uint64_t> m3_2 = { 210, 220 };
    std::pair<uint64_t, uint64_t> m4_1 = { 230, 240 };
    std::pair<uint64_t, uint64_t> m4_2 = { 250, 260 };
    std::pair<uint64_t, uint64_t> m5_1 = { 270, 280 };
    std::pair<uint64_t, uint64_t> m5_2 = { 290, 300 };

    AddressNodePtr res1_1 = MakeReconstructionNode("Ext3", { { c1 } });
    res1_1->m_metadata = { { m1_1 } };

    AddressNodePtr res1_2 = MakeReconstructionNode("Ext3", { { c2 } });
    res1_2->m_metadata = { { m2_1, m2_2 } };

    AddressNodePtr res1_3 = MakeReconstructionNode("Ext3", { { c3 } });
    res1_3->m_metadata = { { m3_1, m3_2 } };

    AddressNodePtr res2_1 = MakeReconstructionNode("Ext4", { { c1 } });
    res2_1->m_metadata = { { m1_2 } };

    AddressNodePtr res2_2 = MakeReconstructionNode("Ext4", { { c4 } });
    res2_2->m_metadata = { { m4_1, m4_2 } };

    AddressNodePtr res2_3 = MakeReconstructionNode("Ext4", { { c5 } });
    res2_3->m_metadata = { { m5_1, m5_2 } };

    AddressNodeList res1 = { res1_1, res1_2, res1_3 };
    AddressNodeList res2 = { res2_1, res2_2, res2_3 };

    // Attach res1 and res2 to the tree root so tree integrity checks can traverse them
    tree->tree->add_children(res1);
    tree->tree->add_children(res2);

    Union op(nullptr, nullptr);
    AddressNodeList output = op.setOperator(res1, res2);

    // Check Output Size (3 from res1 + 3 from res2 - 1 shared match = 5 total outputs)
    EXPECT_EQ(output.size(), 5);

    // Find the matched node (data range == c1)
    auto matched_it = std::find_if(output.begin(), output.end(), [&](const AddressNodePtr& node) {
        return !node->m_data.empty() &&
            !node->m_data.front().empty() &&
            node->m_data.front().front() == c1;
        });

    ASSERT_NE(matched_it, output.end()) << "Matched '+' node for range c1 not found in Union output.";
    AddressNodePtr matched_node = *matched_it;

    // Verify Matched Node Properties
    EXPECT_EQ(matched_node->m_tag, "+");
    EXPECT_EQ(matched_node->m_metadata.size(), 2);
    EXPECT_EQ(matched_node->m_metadata[0].front(), m1_1);
    EXPECT_EQ(matched_node->m_metadata[1].front(), m1_2);

    // Verify DAG Convergence (Matched '+' node must have BOTH parents)
    EXPECT_EQ(matched_node->m_parents.size(), 2);
    EXPECT_EQ(matched_node->m_parents[0], res1_1.get());
    EXPECT_EQ(matched_node->m_parents[1], res2_1.get());

    // Verify Unmatched Nodes carry tag "+" and have 1 parent each
    for (const AddressNodePtr& node : output) {
        EXPECT_EQ(node->m_tag, "+");
        if (node != matched_node) {
            EXPECT_EQ(node->m_parents.size(), 1);
        }
    }

    AssertTreeIntegrity();
}