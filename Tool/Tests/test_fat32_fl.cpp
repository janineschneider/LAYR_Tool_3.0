#include "gtest/gtest.h"
#include "../tree.h"
#include "../Rules/FAT32/fat32_fl.h"
#include "../Rules/FAT32/fat32_fsl.h"
#include "../getworkingdir.h"
#include <cstdint>
#include <string>
#include <fstream>

class FAT32_flRuleTest : public ::testing::Test
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

TEST_F(FAT32_flRuleTest, TestFat32_fl)
{
    std::string imagePath = getTestImagePath("testimage_fat32.dd");

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);
    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList output;
        AddressNodeList output2;

        AddressNodeList input;
        input.push_back(tree.tree);

        Rule* r1;
        r1 = new FAT32_fsl();
        output = r1->evaluate(input);
        std::cout << "r1 completed" << std::endl;

        Rule* r2;
        r2 = new FAT32_fl();
        output2 = r2->evaluate(output);

        tree.print();
        EXPECT_EQ(tree.getNodes("FAT32_fl").size(), 5);
    }
}