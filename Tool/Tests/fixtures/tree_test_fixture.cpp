#include "tree_test_fixture.hpp"
#include <algorithm>

void TreeTestFixture::SetUp()
{
    CreateTestTree();
}

void TreeTestFixture::TearDown()
{
    tree.reset();
    input_data.reset();
    input_stream.reset();
}

void TreeTestFixture::CreateTestTree(const std::string& raw)
{
    input_stream = std::make_unique<std::istringstream>(raw);
    input_data = std::make_unique<RootContainer>(*input_stream);
    tree = std::make_unique<AddressTreeRoot>(input_data.get());

    if (tree && tree->tree) {
        tree->tree->root = tree.get();
        tree->tree->depth = 0;
    }
}

AddressNodePtr TreeTestFixture::MakeReconstructionNode(
    const std::string& tag,
    const std::vector<std::vector<std::pair<uint64_t, uint64_t>>>& data) const
{
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadata;
    auto node = std::make_shared<reconstructionNode>(data, metadata, tag);
    node->root = tree.get();
    return node;
}

AddressNodePtr TreeTestFixture::MakeTransformationNode(
    const std::string& tag,
    const std::vector<std::vector<std::pair<uint64_t, uint64_t>>>& data) const
{
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadata;
    std::vector<unsigned char> testBytes(12, 30);
    auto container = std::make_shared<TransformationContainer>(testBytes);
    auto node = std::make_shared<transformationNode>(data, metadata, tag, container);
    node->root = tree.get();
    return node;
}

void TreeTestFixture::AddLayer1()
{
    AddressNodeList children;

    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> partition0 = { {{0, 1}} };
    children.push_back(MakeReconstructionNode("Partition", partition0));

    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> partition1 = { {{2, 4}} };
    children.push_back(MakeReconstructionNode("Partition", partition1));

    tree->tree->add_children(children);
}

void TreeTestFixture::AddLayer2()
{
    for (auto& partition : *tree->tree->m_children) {
        AddressNodeList children;
        const auto& range = partition->m_data.front().front();
        for (uint64_t i = range.first; i <= range.second; ++i) {
            std::vector<std::vector<std::pair<uint64_t, uint64_t>>> file_data = { {{i, i}} };
            children.push_back(MakeReconstructionNode("File", file_data));
        }
        partition->add_children(children);
    }
}

void TreeTestFixture::AddLayer3()
{
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> zip_data;
    zip_data.push_back({ {0, 10} });
    auto zip_node = MakeTransformationNode("ZIP", zip_data);

    if (tree->tree->m_children->size() >= 2) {
        auto parent = tree->tree->m_children->at(1)->m_children->at(2);
        parent->add_children({ zip_node });
    }
}

std::vector<AddressNode*> TreeTestFixture::CollectNodes(AddressNode* start) const
{
    if (start == nullptr) {
        start = tree->tree.get();
    }

    std::vector<AddressNode*> all;
    all.push_back(start);
    for (auto& child : *start->m_children) {
        auto result = CollectNodes(child.get());
        all.insert(all.end(), result.begin(), result.end());
    }
    return all;
}

uint64_t TreeTestFixture::ComputeMaxDepth(AddressNode* start) const
{
    if (start == nullptr) {
        start = tree->tree.get();
    }
    uint64_t max_depth = start->depth;
    for (auto& child : *start->m_children) {
        max_depth = std::max(max_depth, ComputeMaxDepth(child.get()));
    }
    return max_depth;
}

void TreeTestFixture::AssertTreeIntegrity() const
{
    AssertChildConsistency(tree->tree.get());
    EXPECT_EQ(tree->depth, ComputeMaxDepth());
}

void TreeTestFixture::AssertLayerCount(uint64_t level, size_t expected) const
{
    auto layer = tree->getLayer(level);
    EXPECT_EQ(layer.size(), expected);
}

void TreeTestFixture::AssertChildConsistency(AddressNode* node) const
{
    for (auto& child : *node->m_children) {
        EXPECT_NE(std::find(child->m_parents.begin(), child->m_parents.end(), node), child->m_parents.end())
            << "Node is not listed among its child's m_parents!";
        EXPECT_EQ(child->root, tree.get());
        EXPECT_EQ(child->depth, node->depth + 1);
        AssertChildConsistency(child.get());
    }
}

void AssertValidNodeRanges(AddressNodeList& nodes)
{
    ASSERT_FALSE(nodes.empty()) << "Node list is empty!";

    uint64_t previousEnd = 0;
    bool isFirst = true;

    for (const auto& node : nodes) {
        ASSERT_NE(node, nullptr);
        ASSERT_FALSE(node->m_data.empty());

        auto range = node->m_data.front().front();
        uint64_t start = range.first;
        uint64_t end = range.second;

        EXPECT_LT(start, end)
            << "Invalid range: start (" << start << ") >= end (" << end << ")";

        EXPECT_EQ(start % 512, 0u)
            << "Start address " << start << " is not sector-aligned!";

        if (!isFirst) {
            EXPECT_GT(start, previousEnd)
                << "Node range overlaps with or precedes prior range!";
        }
        previousEnd = end;
        isFirst = false;
    }
}

void AssertValidChildLinkage(const AddressNodePtr& parent)
{
    ASSERT_NE(parent, nullptr) << "Parent node is null!";
    ASSERT_NE(parent->m_children, nullptr) << "Parent's m_children vector is uninitialized!";
    ASSERT_FALSE(parent->m_children->empty()) << "Parent has no attached child nodes!";

    for (const auto& child : *parent->m_children) {
        ASSERT_NE(child, nullptr) << "Encountered a null child node!";

        EXPECT_EQ(child->root, parent->root)
            << "Child node does not point to the correct root context!";

        EXPECT_FALSE(child->m_data.empty())
            << "Child node was attached with an empty data payload!";
    }
}
