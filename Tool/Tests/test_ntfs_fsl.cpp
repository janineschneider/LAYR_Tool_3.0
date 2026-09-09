#include "gtest/gtest.h"
#include "../tree.h"
#include "../Rules/NTFS/ntfs_fsl.h"
#include "../getworkingdir.h"
#include <cstdint>
#include <string>
#include <fstream>

class NtfsfslRuleTest : public ::testing::Test
{
protected:
    std::string getTestImagePath(const std::string& imageName)
    {
        char buffer[1000];
        getCurrentDir(buffer, 1000);
        std::string imagePath(buffer);

        size_t found = imagePath.rfind(slash);
        imagePath.erase(found, imagePath.size() - found);
        imagePath.append(slash + "TestImages" + slash + "NTFS" + slash);
        imagePath.append(imageName);

        return imagePath;
    }
};

TEST_F(NtfsfslRuleTest, OpensTestImage)
{
    std::string imagePath = getTestImagePath("testimage_ntfs.dd");
    std::ifstream imageStream(imagePath, std::ios::binary);
    ASSERT_TRUE(imageStream.is_open()) << "Failed to open test image: " << imagePath;
    imageStream.close();
}

TEST_F(NtfsfslRuleTest, ProducesAtLeastOneNtfsFslNode)
{
    std::string imagePath = getTestImagePath("testimage_ntfs.dd");
    std::ifstream imageStream(imagePath, std::ios::binary);
    ASSERT_TRUE(imageStream.is_open()) << "Failed to open test image: " << imagePath;

    ByteContainer* rawData = new RootContainer(imageStream);
    AddressTreeRoot tree(rawData);
    AddressNodeList input{ tree.tree };
    Rule* r1 = new Ntfs_fsl();
    AddressNodeList output = r1->evaluate(input);

    ASSERT_FALSE(output.empty());
    EXPECT_EQ(output.front()->m_tag, "ntfs_fsl");
    EXPECT_FALSE(output.front()->m_data.empty());
    EXPECT_FALSE(output.front()->m_metadata.empty());
    EXPECT_GT(tree.depth, 0u);
    EXPECT_GT(tree.getNodes("ntfs_fsl").size(), 0u);

    for (const auto& metadata_block : output.front()->m_metadata) {
        for (const auto& range : metadata_block) {
            EXPECT_LT(range.first, range.second);
            EXPECT_LT(range.second, rawData->size());
        }
    }

    delete r1;
    delete rawData;
    imageStream.close();
}

