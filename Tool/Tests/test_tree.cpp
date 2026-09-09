#include "gtest/gtest.h"
#include "../tree.h"
#include "../Rules/DOS/dos.h"
#include "../Rules/FAT32/fat32_fl.h"
#include "../Rules/FAT32/fat32_fsl.h"
#include "../SequentialOperators/seqcomp.h"
#include "../getworkingdir.h"
#include <cstdint>
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>

void add_layer1(AddressTreeRoot* tree)
{
    std::string tag = "Partition";

    AddressNodeList children;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> node_data;
    std::vector<std::pair<uint64_t, uint64_t>> temp_node_data;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> node_metadata;
    temp_node_data.push_back(std::pair<uint64_t, uint64_t>(0, 1));
    node_data.push_back(temp_node_data);
    children.push_back(std::make_shared<reconstructionNode>(node_data, node_metadata, tag));
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> node_data2;
    std::vector<std::pair<uint64_t, uint64_t>> temp_node_data2;
    temp_node_data2.push_back(std::pair<uint64_t, uint64_t>(2, 4));
    node_data2.push_back(temp_node_data2);
    children.push_back(std::make_shared<reconstructionNode>(node_data2, node_metadata, tag));

    tree->tree->add_children(std::move(children));
}

void add_layer2(AddressTreeRoot* tree)
{
    std::string tag = "File";
    for (std::shared_ptr<AddressNode>& child : *tree->tree->m_children) {
        std::vector<std::shared_ptr<AddressNode>> children;
        for (uint64_t i = child->m_data.begin()->begin()->first; i <= child->m_data.begin()->begin()->second; i++) {
            std::vector<std::pair<uint64_t, uint64_t>> temp_node_data;
            std::vector<std::vector<std::pair<uint64_t, uint64_t>>> node_data;
            std::vector<std::vector<std::pair<uint64_t, uint64_t>>> node_metadata;
            temp_node_data.push_back(std::pair(i, i));
            node_data.push_back(temp_node_data);
            children.emplace_back(std::shared_ptr<AddressNode>(new reconstructionNode(node_data, node_metadata, tag)));
        }
        child->add_children(std::move(children));
    }
}

void add_layer3(AddressTreeRoot* tree)
{
    std::string tag = "ZIP";
    std::vector<std::shared_ptr<AddressNode>> children;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> node_data;
    std::vector<std::pair<uint64_t, uint64_t>> temp_node_data;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> node_metadata;
    temp_node_data.push_back(std::pair(0, 10));
    node_data.push_back(temp_node_data);

    std::vector<unsigned char> testBytes(11, 'X');
    auto container = std::make_shared<TransformationContainer>(testBytes);

    children.push_back(std::make_shared<transformationNode>(node_data, node_metadata, tag, container));
    tree->tree->m_children->at(1)->m_children->at(2)->add_children(std::move(children));
}

class AddressTreeTest : public ::testing::Test
{
protected:
    std::unique_ptr<AddressTreeRoot> tree;

    void SetUp() override
    {
        std::string rawdata = "12345";
        std::unique_ptr<std::stringbuf> buffer = std::make_unique<std::stringbuf>(rawdata, std::ios::in);
        std::istream input(buffer.get());
        RootContainer testdata(input);
        tree = std::make_unique<AddressTreeRoot>(&testdata);
    }
};

TEST_F(AddressTreeTest, Layer1AddsPartitions)
{
    add_layer1(tree.get());
    tree->print();
    std::vector<AddressNode*> layer1 = tree->getLayer(1);
    EXPECT_EQ(tree->depth, 1);
    ASSERT_EQ(layer1.size(), 2);
    EXPECT_EQ(layer1[0]->m_data.begin()->begin()->first, 0);
    EXPECT_EQ(layer1[0]->m_data.begin()->begin()->second, 1);
    EXPECT_EQ(layer1[0]->m_tag, "Partition");
    EXPECT_EQ(layer1[1]->m_data.begin()->begin()->first, 2);
    EXPECT_EQ(layer1[1]->m_data.begin()->begin()->second, 4);
    EXPECT_EQ(layer1[1]->m_tag, "Partition");
}

TEST_F(AddressTreeTest, Layer2AddsFilesToPartitions)
{
    add_layer1(tree.get());
    add_layer2(tree.get());
    EXPECT_EQ(tree->depth, 2);

    tree->print();
    std::vector<AddressNode*> layer2 = tree->getLayer(2);
    ASSERT_EQ(layer2.size(), 5);  // 2 from [0,1] and 3 from [2,4]

    for (AddressNode* node : layer2) {
        EXPECT_EQ(node->m_data.begin()->begin()->first, node->m_data.begin()->begin()->second);  // should be individual addresses
        EXPECT_EQ(node->m_tag, "File");
    }
}

TEST_F(AddressTreeTest, Layer3AddsZipToSpecificNode)
{
    add_layer1(tree.get());
    add_layer2(tree.get());
    add_layer3(tree.get());
    EXPECT_EQ(tree->depth, 3);
    tree->print();
    // Should be at Partition 2 (index 1), File node for address 2 (index 2)
    AddressNodePtr zip_parent = tree->tree->m_children->at(1)->m_children->at(2);
    ASSERT_FALSE(zip_parent->m_children == nullptr);
    ASSERT_EQ(zip_parent->m_children->size(), 1);
    EXPECT_EQ(zip_parent->m_children->at(0)->m_tag, "ZIP");
    EXPECT_EQ(zip_parent->m_children->at(0)->m_data.begin()->begin()->first, 0);
    EXPECT_EQ(zip_parent->m_children->at(0)->m_data.begin()->begin()->second, 10);
}

TEST_F(AddressTreeTest, GetAllFilesReturnsCorrectCount)
{
    add_layer1(tree.get());
    add_layer2(tree.get());
    std::vector<AddressNode*> files = tree->getNodes("File");
    ASSERT_EQ(files.size(), 5);  // Files from 0 to 4
}

TEST_F(AddressTreeTest, GetNodesInRangeReturnsCorrectSubset)
{
    add_layer1(tree.get());
    add_layer2(tree.get());
    std::vector<AddressNode*> range_nodes = tree->getNodes(std::pair<uint64_t, uint64_t>{2, 3});
    // Expected: nodes with address 2 and 3
    ASSERT_EQ(range_nodes.size(), 2);
    for (auto* node : range_nodes) {
        EXPECT_GE(node->m_data.begin()->begin()->first, 2);
        EXPECT_LE(node->m_data.begin()->begin()->second, 3);
    }
}