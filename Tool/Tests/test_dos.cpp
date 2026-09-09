
#include "../getworkingdir.h"
#include "gtest/gtest.h"
#include "../tree.h"
#include "../Rules/DOS/dos.h"
#include <cstdint>
#include <string>
#include <fstream>

class DOSRuleTest : public ::testing::Test
{
protected:
    std::string getTestImagePath(const std::string& imageName)
    {
        char buffer[1000];
        getCurrentDir(buffer, 1000);
        std::string imagePath(buffer);

        size_t found = imagePath.rfind(slash);
        imagePath.erase(found, imagePath.size() - found);
        imagePath.append(slash + "TestImages" + slash + "DOS" + slash);
        imagePath.append(imageName);

        return imagePath;
    }
};

TEST_F(DOSRuleTest, TestDosRule)
{
    std::string imagePath = getTestImagePath("ext-part-test-2.dd");

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);
    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);
        AddressTreeRoot tree(rawData);
        AddressNodeList output;

        Rule* r1;
        r1 = new DOS();
        AddressNodeList in_nodes;
        in_nodes.push_back(tree.tree);
        output = r1->evaluate(in_nodes);
        tree.print();
        imageStream.close();
        ASSERT_EQ(output.size(), 6);
        ASSERT_EQ(tree.depth, 1);
        EXPECT_EQ(tree.tree->m_data.front().front().first, 0);
        EXPECT_EQ(tree.tree->m_data.front().front().second, 159989760ULL);
        EXPECT_EQ(tree.tree->m_children->size(), 6);
    }