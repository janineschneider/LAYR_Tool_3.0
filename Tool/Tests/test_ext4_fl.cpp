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

class Ext4_flRuleTest : public TreeTestFixture
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

    // Creates a reconstruction node containing both content data and metadata ranges.
    AddressNodePtr BuildContentAndMetadataNode(
        const std::string& raw,
        std::pair<uint64_t, uint64_t> superBlockRange,
        std::pair<uint64_t, uint64_t> inodeTableRange)
    {
        AddressNodePtr node = BuildContentOnlyNode(raw);
        node->m_metadata = { { superBlockRange }, { inodeTableRange } };
        return node;
    }
};

// Verifies that Ext4_fl correctly parses a direct block pointer inode.
TEST_F(Ext4_flRuleTest, Ext4FlParsesDirectBlockInode)
{
    const std::pair<uint64_t, uint64_t> superBlockRange = { 1024, 2047 };
    const std::pair<uint64_t, uint64_t> inodeTableRange = { 2048, 5119 };
    const uint64_t inodeAbs = 4608;

    std::string raw(5120, '\x00');

    PutU32LE(raw, superBlockRange.first + 0x18, 0);
    PutU16LE(raw, superBlockRange.first + 0x58, 256);

    PutU16LE(raw, inodeAbs + 0x0, 0x8180);
    PutU32LE(raw, inodeAbs + 0x4, 1000);
    PutU32LE(raw, inodeAbs + 0x20, 0);
    PutU32LE(raw, inodeAbs + 0x6C, 0);
    PutU32LE(raw, inodeAbs + 40, 3);

    AddressNodePtr node = BuildContentAndMetadataNode(raw, superBlockRange, inodeTableRange);

    Rule* carve = new Ext4_fl();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);

    auto blockRange = result[0]->m_data.front().front();
    EXPECT_EQ(blockRange.first, 3072u);
    EXPECT_EQ(blockRange.second, 4071u);

    auto inodeMeta = result[0]->m_metadata.front().front();
    EXPECT_EQ(inodeMeta.first, inodeAbs);
    EXPECT_EQ(inodeMeta.second, inodeAbs + 256 - 1);

    delete carve;
}

// Verifies that Ext4_fl correctly parses an inode containing an extent tree leaf.
TEST_F(Ext4_flRuleTest, Ext4FlParsesExtentTreeLeafInode)
{
    const std::pair<uint64_t, uint64_t> superBlockRange = { 1024, 2047 };
    const std::pair<uint64_t, uint64_t> inodeTableRange = { 2048, 5119 };
    const uint64_t inodeAbs = 4608;
    const uint64_t extentHeaderAbs = inodeAbs + 0x28;

    std::string raw(5120, '\x00');

    PutU32LE(raw, superBlockRange.first + 0x18, 0);
    PutU16LE(raw, superBlockRange.first + 0x58, 256);

    PutU16LE(raw, inodeAbs + 0x0, 0x8180);
    PutU32LE(raw, inodeAbs + 0x4, 1024);
    PutU32LE(raw, inodeAbs + 0x20, 0x80000);
    PutU32LE(raw, inodeAbs + 0x6C, 0);

    PutU16LE(raw, extentHeaderAbs + 0x0, 0xF30A);
    PutU16LE(raw, extentHeaderAbs + 0x2, 1);
    PutU16LE(raw, extentHeaderAbs + 0x4, 4);
    PutU16LE(raw, extentHeaderAbs + 0x6, 0);

    const uint64_t extentAbs = extentHeaderAbs + 0xc;
    PutU32LE(raw, extentAbs + 0x4, 1);
    PutU32LE(raw, extentAbs + 0x8, 10);

    AddressNodePtr node = BuildContentAndMetadataNode(raw, superBlockRange, inodeTableRange);

    Rule* carve = new Ext4_fl();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);

    auto blockRange = result[0]->m_data.front().front();
    EXPECT_EQ(blockRange.first, 10240u);
    EXPECT_EQ(blockRange.second, 11263u);

    delete carve;
}

// Verifies that Ext4_fl ignores reserved and unallocated empty inodes without creating child nodes.
TEST_F(Ext4_flRuleTest, Ext4FlSkipsReservedAndEmptyInodesProducesNoChildren)
{
    const std::pair<uint64_t, uint64_t> superBlockRange = { 1024, 2047 };
    const std::pair<uint64_t, uint64_t> inodeTableRange = { 2048, 5119 };

    std::string raw(5120, '\x00');
    PutU32LE(raw, superBlockRange.first + 0x18, 0);
    PutU16LE(raw, superBlockRange.first + 0x58, 256);

    AddressNodePtr node = BuildContentAndMetadataNode(raw, superBlockRange, inodeTableRange);

    Rule* carve = new Ext4_fl();
    AddressNodeList result = carve->evaluate({ node });

    EXPECT_TRUE(result.empty());
    delete carve;
}