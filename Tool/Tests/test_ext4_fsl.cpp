#include "fixtures/tree_test_fixture.hpp"
#include "../Rules/Ext4/ext4_fsl.h"
#include "../Rules/Ext4/ext4_fl.h"

#include <string>
#include <cstdint>

// Writes a 16-bit little-endian integer to the buffer at the specified offset.
static void PutU16LE(std::string& buf, size_t offset, uint16_t value)
{
    buf[offset] = static_cast<char>(value & 0xFF);
    buf[offset + 1] = static_cast<char>((value >> 8) & 0xFF);
}

// Writes a 32-bit little-endian integer to the buffer at the specified offset.
static void PutU32LE(std::string& buf, size_t offset, uint32_t value)
{
    buf[offset] = static_cast<char>(value & 0xFF);
    buf[offset + 1] = static_cast<char>((value >> 8) & 0xFF);
    buf[offset + 2] = static_cast<char>((value >> 16) & 0xFF);
    buf[offset + 3] = static_cast<char>((value >> 24) & 0xFF);
}

class Ext4_fslRuleTest : public TreeTestFixture
{
protected:
    // Creates a reconstruction node containing only raw content data.
    AddressNodePtr BuildContentOnlyNode(const std::string& raw)
    {
        CreateTestTree(raw);
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> content = {
            { {0, raw.empty() ? 0 : raw.size() - 1} }
        };
        return MakeReconstructionNode("Range", content);
    }
};

// Verifies that Ext4_fsl correctly parses a superblock and a single 32-bit group descriptor.
TEST_F(Ext4_fslRuleTest, Ext4FslParsesSuperblockAndSingleGroupDescriptor)
{
    const uint64_t partitionStart = 0;
    const uint64_t superBlockStart = partitionStart + 1024;

    std::string raw(2080, '\x00');

    PutU32LE(raw, superBlockStart + 0x18, 0);
    PutU32LE(raw, superBlockStart + 0x4, 8);
    PutU32LE(raw, superBlockStart + 0x20, 8192);
    PutU32LE(raw, superBlockStart + 0x28, 8);
    PutU16LE(raw, superBlockStart + 0x58, 256);
    PutU32LE(raw, superBlockStart + 0x60, 0);

    const uint64_t groupDescStart = superBlockStart + 1024;
    PutU32LE(raw, groupDescStart + 0x8, 5);

    AddressNodePtr node = BuildContentOnlyNode(raw);

    Rule* carve = new Ext4_fsl();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto content = result[0]->m_data.front().front();
    EXPECT_EQ(content.first, 0u);
    EXPECT_EQ(content.second, 8191u);

    ASSERT_EQ(result[0]->m_metadata.size(), 2u);

    auto superBlockOut = result[0]->m_metadata[0].front();
    EXPECT_EQ(superBlockOut.first, 1024u);
    EXPECT_EQ(superBlockOut.second, 2047u);

    auto inodeTableOut = result[0]->m_metadata[1].front();
    EXPECT_EQ(inodeTableOut.first, 5120u);
    EXPECT_EQ(inodeTableOut.second, 7167u);

    delete carve;
}

// Verifies the metadata behavior when parsing Ext4 64-bit group descriptors.
TEST_F(Ext4_fslRuleTest, Ext4Fsl64BitDescriptorsProduceDuplicatedMetadataPair)
{
    const uint64_t partitionStart = 0;
    const uint64_t superBlockStart = partitionStart + 1024;

    std::string raw(2112, '\x00');

    PutU32LE(raw, superBlockStart + 0x18, 0);
    PutU32LE(raw, superBlockStart + 0x4, 8);
    PutU32LE(raw, superBlockStart + 0x20, 8192);
    PutU32LE(raw, superBlockStart + 0x28, 8);
    PutU16LE(raw, superBlockStart + 0x58, 256);
    PutU32LE(raw, superBlockStart + 0x60, 0x80);
    PutU16LE(raw, superBlockStart + 0xFE, 64);

    const uint64_t groupDescStart = superBlockStart + 1024;
    PutU32LE(raw, groupDescStart + 0x8, 5);
    PutU32LE(raw, groupDescStart + 0x28, 0);

    AddressNodePtr node = BuildContentOnlyNode(raw);

    Rule* carve = new Ext4_fsl();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    ASSERT_EQ(result[0]->m_metadata.size(), 2u);

    ASSERT_EQ(result[0]->m_metadata[1].size(), 2u);
    for (const auto& pair : result[0]->m_metadata[1]) {
        EXPECT_EQ(pair.first, 5120u);
        EXPECT_EQ(pair.second, 7167u);
    }

    delete carve;
}

// Verifies that unsupported meta block group (meta_bg) filesystems throw an exception.
TEST_F(Ext4_fslRuleTest, Ext4FslMetaBlockGroupsThrowsUnsupportedError)
{
    const uint64_t partitionStart = 0;
    const uint64_t superBlockStart = partitionStart + 1024;

    std::string raw(2080, '\x00');
    PutU32LE(raw, superBlockStart + 0x18, 0);
    PutU32LE(raw, superBlockStart + 0x4, 8);
    PutU32LE(raw, superBlockStart + 0x20, 8192);
    PutU32LE(raw, superBlockStart + 0x28, 8);
    PutU16LE(raw, superBlockStart + 0x58, 256);
    PutU32LE(raw, superBlockStart + 0x60, 0x10);

    AddressNodePtr node = BuildContentOnlyNode(raw);

    Rule* carve = new Ext4_fsl();
    EXPECT_THROW(carve->evaluate({ node }), const char*);
    delete carve;
}