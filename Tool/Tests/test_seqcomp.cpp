#include "gtest/gtest.h"
#include "../getworkingdir.h"
#include "../Rules/FAT32/fat32_fl.h"
#include "../Rules/FAT32/fat32_fsl.h"
#include "../SequentialOperators/seqcomp.h"
#include "../tree.h"
#include <vector>
#include <memory>
#include <fstream>

AddressNodeList choose(AddressNodeList input)
{
    AddressNodeList result;
    result.push_back(input.at(0));
    return result;
}

class SeqCompTest : public ::testing::Test
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

TEST_F(SeqCompTest, SeqCompMultiChoice)
{
    char buffer[1000];
    getCurrentDir(buffer, 1000);
    std::string imagePath(buffer);

    //Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    imagePath.append(slash + "TestImages" + slash + "FAT32" + slash);

    //Read in image name
    std::string imageName;
    imageName = "testimage_fat32.dd";
    imagePath.append(imageName);

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);
    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList output;
        AddressNodeList output2;
        Rule* h1;
        h1 = new FAT32_fsl();
        AddressNodeList in_nodes;
        in_nodes.push_back(tree.tree);
        Rule* h2;
        h2 = new FAT32_fl();
        SeqCompMultiChoice op(h1, h2, choose);
        output = op.evaluate(in_nodes);

        tree.print();
        EXPECT_EQ(tree.getNodes("FAT32_fl").size(), 5);
    }
}
TEST_F(SeqCompTest, SeqCompMulti)
{
    char buffer[1000];
    getCurrentDir(buffer, 1000);
    std::string imagePath(buffer);

    //Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    imagePath.append(slash + "TestImages" + slash + "FAT32" + slash);

    //Read in image name
    std::string imageName;
    imageName = "testimage_fat32.dd";
    imagePath.append(imageName);

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);
    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList output;
        AddressNodeList output2;
        Rule* h1;
        h1 = new FAT32_fsl();
        AddressNodeList in_nodes;
        in_nodes.push_back(tree.tree);
        Rule* h2;
        h2 = new FAT32_fl();
        SeqCompMulti op(h1, h2, choose);
        output = op.evaluate(in_nodes);

        tree.print();
        EXPECT_EQ(tree.getNodes("FAT32_fl").size(), 5);
    }
}

TEST_F(SeqCompTest, SeqCompSingleChoice)
{
    char buffer[1000];
    getCurrentDir(buffer, 1000);
    std::string imagePath(buffer);

    //Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    imagePath.append(slash + "TestImages" + slash + "FAT32" + slash);

    //Read in image name
    std::string imageName;
    imageName = "testimage_fat32.dd";
    imagePath.append(imageName);

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);
    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList output;
        AddressNodeList output2;
        Rule* h1;
        h1 = new FAT32_fsl();
        AddressNodeList in_nodes;
        in_nodes.push_back(tree.tree);
        Rule* h2;
        h2 = new FAT32_fl();
        SeqCompSingleChoice op(h1, h2, choose);
        output = op.evaluate(in_nodes);

        tree.print();
        EXPECT_EQ(tree.getNodes("FAT32_fl").size(), 5);
    }
}

TEST_F(SeqCompTest, SeqCompSingle)
{
    char buffer[1000];
    getCurrentDir(buffer, 1000);
    std::string imagePath(buffer);

    //Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    imagePath.append(slash + "TestImages" + slash + "FAT32" + slash);

    //Read in image name
    std::string imageName;
    imageName = "testimage_fat32.dd";
    imagePath.append(imageName);

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);
    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList output;
        AddressNodeList output2;
        Rule* h1;
        h1 = new FAT32_fsl();
        AddressNodeList in_nodes;
        in_nodes.push_back(tree.tree);
        Rule* h2;
        h2 = new FAT32_fl();
        SeqCompSingle op(h1, h2);
        output = op.evaluate(in_nodes);

        tree.print();
        EXPECT_EQ(tree.getNodes("FAT32_fl").size(), 5);
    }
}