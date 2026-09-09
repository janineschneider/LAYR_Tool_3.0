#include "gtest/gtest.h"
#include "../tree.h"
#include "../Rules/Ext3/ext3_fl.h"
#include "../Rules/Ext3/ext3_fsl.h"
#include "../getworkingdir.h"
#include <cstdint>
#include <string>
#include <fstream>
#include <algorithm>

class EXT3_fslRuleTest : public ::testing::Test
{
protected:
    std::string getTestImagePath(const std::string& imageName)
    {
        char buffer[1000];
        getCurrentDir(buffer, 1000);
        std::string imagePath(buffer);

        size_t found = imagePath.rfind(slash);
        imagePath.erase(found, imagePath.size() - found);
        imagePath.append(slash + "TestImages" + slash + "EXT3" + slash);
        imagePath.append(imageName);

        return imagePath;
    }

    // Recursively validates parent-child links, node root context, and relative depths
    void AssertChildConsistency(AddressNode* node, AddressTreeRoot* expectedRoot)
    {
        ASSERT_NE(node, nullptr);
        ASSERT_NE(node->m_children, nullptr);

        for (auto& child : *node->m_children) {
            ASSERT_NE(child, nullptr);

            auto it = std::find(child->m_parents.begin(), child->m_parents.end(), node);
            EXPECT_NE(it, child->m_parents.end())
                << "Node [" << child->m_tag << "] is missing parent pointer back to [" << node->m_tag << "]!";

            EXPECT_EQ(child->root, expectedRoot)
                << "Child node [" << child->m_tag << "] does not match expected AddressTreeRoot context!";

            EXPECT_EQ(child->depth, node->depth + 1)
                << "Child node [" << child->m_tag << "] depth mismatch!";

            AssertChildConsistency(child.get(), expectedRoot);
        }
    }
};

TEST_F(EXT3_fslRuleTest, TestExt3_fsl)
{
    std::string imagePath = getTestImagePath("ext3-img-kw-1.dd");
    std::ifstream imageStream(imagePath, std::ios::binary);
    ASSERT_TRUE(imageStream.is_open()) << "Failed to open image at: " << imagePath;

    ByteContainer* rawData = new RootContainer(imageStream);
    AddressTreeRoot tree(rawData);

    if (tree.tree) {
        tree.tree->root = &tree;
        tree.tree->depth = 0;
    }

    Rule* h1 = new Ext3_fsl();
    AddressNodeList input = { tree.tree };

    AddressNodeList output = h1->evaluate(input);

    ASSERT_FALSE(output.empty());

    EXPECT_EQ(tree.tree->m_children->size(), 1u);

    AssertChildConsistency(tree.tree.get(), &tree);

    for (const auto& childNode : *tree.tree->m_children) {
        EXPECT_EQ(childNode->m_tag, "ext3_fsl");
        EXPECT_FALSE(childNode->m_data.empty());
        EXPECT_FALSE(childNode->m_metadata.empty());
    }

    delete h1;
    delete rawData;
    imageStream.close();
}