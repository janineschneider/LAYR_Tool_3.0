#include "gtest/gtest.h"
#include "../tree.h"
#include "../Rules/Ext3/ext3_fl.h"
#include "../Rules/Ext3/ext3_fsl.h"
#include "../getworkingdir.h"
#include <cstdint>
#include <string>
#include <fstream>

class EXT3_flRuleTest : public ::testing::Test
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
};

TEST_F(EXT3_flRuleTest, TestExt3_fl)
{
    std::string imagePath = getTestImagePath("ext3-img-kw-1.dd");

    std::ifstream imageStream(imagePath, std::ios::binary);
    ASSERT_TRUE(imageStream.is_open()) << "Failed to open image at: " << imagePath;

    ByteContainer* rawData = new RootContainer(imageStream);
    AddressTreeRoot tree(rawData);

    Rule* r1 = new Ext3_fsl();
    AddressNodeList in_nodes = { tree.tree };
    AddressNodeList output = r1->evaluate(in_nodes);

    ASSERT_EQ(output.size(), 1u);
    EXPECT_EQ(tree.getNodes("ext3_fsl").size(), 1u);

    Rule* r2 = new Ext3_fl();
    AddressNodeList output2 = r2->evaluate(output);

    ASSERT_EQ(output2.size(), 6u);

    tree.print();

    EXPECT_EQ(tree.getNodes("ext3_fsl").size(), 1u);
    EXPECT_EQ(tree.getNodes("ext3_fl").size(), 6u);

    for (const auto& node : output2) {
        EXPECT_NE(node, nullptr);
        EXPECT_EQ(node->m_tag, "ext3_fl");
        EXPECT_FALSE(node->m_data.empty());
    }

    delete r1;
    delete r2;
    delete rawData;
    imageStream.close();
}