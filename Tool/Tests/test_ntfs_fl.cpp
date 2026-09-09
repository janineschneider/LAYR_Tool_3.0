#include "gtest/gtest.h"
#include "../tree.h"
#include "../Rules/NTFS/ntfs_fl.h"
#include "../Rules/NTFS/ntfs_fsl.h"
#include "../getworkingdir.h"
#include <cstdint>
#include <string>
#include <fstream>

class NtfsflRuleTest : public ::testing::Test
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

TEST_F(NtfsflRuleTest, ProducesAtLeastOneNtfsFlNode)
{
    std::string imagePath = getTestImagePath("testimage_ntfs.dd");
    std::ifstream imageStream(imagePath, std::ios::binary);
    ASSERT_TRUE(imageStream.is_open()) << "Failed to open test image: " << imagePath;

    ByteContainer* rawData = new RootContainer(imageStream);
    AddressTreeRoot tree(rawData);
    AddressNodeList input{ tree.tree };

    Rule* r1 = new Ntfs_fsl();
    AddressNodeList fslOutput = r1->evaluate(input);
    ASSERT_FALSE(fslOutput.empty());

    Rule* r2 = new Ntfs_fl();
    AddressNodeList flOutput = r2->evaluate(fslOutput);

    ASSERT_FALSE(flOutput.empty());

    EXPECT_TRUE(flOutput.front()->m_tag.rfind("ntfs_fl", 0) == 0);
    EXPECT_FALSE(flOutput.front()->m_data.empty());
    EXPECT_FALSE(flOutput.front()->m_metadata.empty());
    EXPECT_GT(tree.depth, 1u);

    for (const auto& block : flOutput.front()->m_data) {
        for (const auto& range : block) {
            EXPECT_LT(range.first, range.second);
            EXPECT_LT(range.second, rawData->size());
        }
    }

    delete r1;
    delete r2;
    delete rawData;
    imageStream.close();
}

TEST_F(NtfsflRuleTest, FlOutputNodesHaveValidMetadataRanges)
{
    std::string imagePath = getTestImagePath("testimage_ntfs.dd");
    std::ifstream imageStream(imagePath, std::ios::binary);
    ASSERT_TRUE(imageStream.is_open()) << "Failed to open test image: " << imagePath;

    ByteContainer* rawData = new RootContainer(imageStream);
    AddressTreeRoot tree(rawData);
    AddressNodeList input{ tree.tree };

    Rule* r1 = new Ntfs_fsl();
    AddressNodeList fslOutput = r1->evaluate(input);
    ASSERT_FALSE(fslOutput.empty());

    Rule* r2 = new Ntfs_fl();
    AddressNodeList flOutput = r2->evaluate(fslOutput);
    ASSERT_FALSE(flOutput.empty());

    for (const auto& node : flOutput) {
        EXPECT_TRUE(node->m_tag.rfind("ntfs_fl", 0) == 0);
        EXPECT_FALSE(node->m_metadata.empty());
        EXPECT_FALSE(node->m_data.empty());
        for (const auto& metadata_block : node->m_metadata) {
            for (const auto& range : metadata_block) {
                EXPECT_LT(range.first, range.second);
                EXPECT_LT(range.second, rawData->size());
            }
        }
    }

    delete r1;
    delete r2;
    delete rawData;
    imageStream.close();
}