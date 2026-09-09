#include "gtest/gtest.h"
#include "fixtures/tree_test_fixture.hpp"
#include "../tree.h"
#include "../ParallelOperators/minus.h"

class MinusTestFixture : public TreeTestFixture
{
protected:
    void SetUp() override
    {
        CreateTestTree("some random input raw string");
    }
};

TEST_F(MinusTestFixture, ExactMatchingContentIsExcluded)
{
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> data1 = { { {10, 20}, {30, 40} } };
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> meta1;

    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> data2 = { { {10, 20}, {30, 40} } };
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> meta2;

    AddressNodePtr node1 = std::make_shared<AddressNode>(data1, meta1, "Ext3");
    node1->root = tree.get();

    AddressNodePtr node2 = std::make_shared<AddressNode>(data2, meta2, "Ext4");
    node2->root = tree.get();

    AddressNodeList res1 = { node1 };
    AddressNodeList res2 = { node2 };

    Minus op(nullptr, nullptr);
    AddressNodeList output = op.setOperator(res1, res2);

    EXPECT_TRUE(output.empty());
}

TEST_F(MinusTestFixture, UnequalContentIsKeptIntact)
{
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> data1 = { { {10, 20}, {30, 40} } };
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> meta1;

    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> data3 = { { {10, 20}, {50, 60} } };
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> meta3;

    AddressNodePtr node1 = std::make_shared<AddressNode>(data1, meta1, "Ext3");
    node1->root = tree.get();

    AddressNodePtr node3 = std::make_shared<AddressNode>(data3, meta3, "Ext4");
    node3->root = tree.get();

    AddressNodeList res1 = { node1 };
    AddressNodeList res2 = { node3 };

    Minus op(nullptr, nullptr);
    AddressNodeList output = op.setOperator(res1, res2);

    ASSERT_EQ(output.size(), 1);
    ASSERT_EQ(output[0]->m_data.size(), 1);
    ASSERT_EQ(output[0]->m_data.front().size(), 2);
    EXPECT_EQ(output[0]->m_data.front()[0].first, 10);
    EXPECT_EQ(output[0]->m_data.front()[0].second, 20);
    EXPECT_EQ(output[0]->m_data.front()[1].first, 30);
    EXPECT_EQ(output[0]->m_data.front()[1].second, 40);
}