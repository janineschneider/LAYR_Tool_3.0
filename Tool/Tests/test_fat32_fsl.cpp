#include "gtest/gtest.h"
#include "../tree.h"
#include "../Rules/FAT32/fat32_fl.h"
#include "../Rules/FAT32/fat32_fsl.h"
#include "../getworkingdir.h"
#include <cstdint>
#include <string>
#include <fstream>

class FAT32_fslRuleTest : public ::testing::Test
{
protected:
    std::string getTestImagePath(const std::string& imageName)
    {
        char buffer[1000];
        getCurrentDir(buffer, 1000);
        std::string imagePath(buffer);

        size_t found = imagePath.rfind(slash);
        imagePath.erase(found, imagePath.size() - found);
        imagePath.append(slash + "TestImages" + slash + "FAT32" + slash);
        imagePath.append(imageName);

        return imagePath;
    }
};


TEST_F(FAT32_fslRuleTest, TestFat32_fsl)
{
    std::string imagePath = getTestImagePath("testimage_fat32.dd");

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);
    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        std::vector<std::pair<uint64_t, uint64_t>> helper;
        helper.push_back(std::make_pair(0, rawData->size()));
        AddressNodeList output;

        Rule* h1;
        h1 = new FAT32_fsl();
        AddressNodeList in_nodes;
        in_nodes.push_back(tree.tree);
        output = h1->evaluate(in_nodes);
        tree.print();
        ASSERT_EQ(tree.depth, 1);
        EXPECT_EQ(tree.tree->m_children->size(), 1);
        imageStream.close();
    }
}