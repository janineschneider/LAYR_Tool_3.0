#include "gtest/gtest.h"
#include "../tree.h"
#include "fixtures/tree_test_fixture.hpp"
#include "../Rules/Carve/carve.h"

#include <string>


// Byte building blocks for the signatures
static const std::string kGif87Header = std::string("\x47\x49\x46\x38\x37\x61", 6); // GIF87a
static const std::string kGif87Footer = std::string("\x00\x3B", 2);
static const std::string kGif89Header = std::string("\x47\x49\x46\x38\x39\x61", 6); // GIF89a
static const std::string kGif89Footer = std::string("\x00\x00\x3B", 3);

static const std::string kJpgHeader = std::string("\xFF\xD8\xFF\xE0\x00\x10", 6);
static const std::string kJpgFooter = std::string("\xFF\xD9", 2);

static const std::string kPngHeader = std::string("\x50\x4E\x47\x00", 4); // 4th byte is wildcard
static const std::string kPngFooter = std::string("\xFF\xFC\xFD\xFE", 4);

static const std::string kTifHeaderIntel = std::string("\x49\x49\x2A\x00", 4);
static const std::string kTifHeaderMotorola = std::string("\x4D\x4D\x00\x2A", 4);

static const std::string kAviHeader =
std::string("RIFF") + std::string("\x00\x00\x00\x00", 4) + std::string("AVI");

static const std::string kMpgHeaderBA = std::string("\x00\x00\x01\xBA", 4);
static const std::string kMpgFooterB9 = std::string("\x00\x00\x01\xB9", 4);
static const std::string kMpgHeaderB3 = std::string("\x00\x00\x01\xB3", 4);
static const std::string kMpgFooterB7 = std::string("\x00\x00\x01\xB7", 4);

static const std::string kFwsHeader = "FWS";

static const std::string kWavHeader =
std::string("RIFF") + std::string("\x00\x00\x00\x00", 4) + std::string("WAVE");

static const std::string kDocShortHeader = std::string("\xD0\xCF\x11\xE0\xA1\xB1", 6);
static const std::string kDocNextHeader =
std::string("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1\x00\x00", 10);

static const std::string kPstHeader = std::string("\x21\x42\x4E\xA5\x6F\xB5\xA6", 7);
static const std::string kOstHeader = std::string("\x21\x42\x44\x4E", 4);
static const std::string kDbxHeader = std::string("\xCF\xAD\x12\xFE\xC5\xFD\x74\x6F", 8);
static const std::string kIdxHeader = std::string("\x4A\x4D\x46\x39", 4);
static const std::string kMbxHeader = std::string("\x4A\x4D\x46\x36", 4);

static const std::string kHtmHeader = "<html";
static const std::string kHtmFooter = "</html>";

static const std::string kPdfHeader = "%PDF";
static const std::string kPdfFooterCR = std::string("%%EOF\x0D", 6);
static const std::string kPdfFooterLF = std::string("%%EOF\x0A", 6);

static const std::string kZipHeader = std::string("PK\x03\x04", 4);
static const std::string kZipFooter = std::string("\x3C\xAC", 2);

static const std::string kJavaHeader = std::string("\xCA\xFE\xBA\xBE", 4);
static const std::string kTgzHeader = std::string("\x1F\x8B\x08\x08", 4);

static const std::string kOggPattern = std::string("\x4F\x67\x67\x53\x00\x02", 6);

class CarveRuleTest : public TreeTestFixture
{
protected:
    AddressNodePtr BuildFullRangeNode(const std::string& raw)
    {
        static constexpr size_t kEofPadding = 64;
        std::string padded = raw + std::string(kEofPadding, '\xFF');
        CreateTestTree(padded);

        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> full_range = {
            { { 0, raw.empty() ? 0 : raw.size() - 1 } }
        };
        return MakeReconstructionNode("Range", full_range);
    }
};

TEST_F(CarveRuleTest, FindsGif87aWithFooter)
{
    std::string raw = kGif87Header + std::string(5, '\x00') + kGif87Footer;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsGif89aWithFooter)
{
    std::string raw = kGif89Header + std::string(5, '\x00') + kGif89Footer;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsJpgWithFooter)
{
    std::string raw = kJpgHeader + std::string(20, '\xAB') + kJpgFooter;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsPngWithFooter)
{
    std::string raw = kPngHeader + std::string(5, '\x00') + kPngFooter;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsTifIntelWithoutFooter)
{
    std::string raw = kTifHeaderIntel + std::string(10, '\xAB');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsTifMotorolaWithoutFooter)
{
    std::string raw = kTifHeaderMotorola + std::string(10, '\xAB');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsOverlappingHeaderInsideAnotherCarve)
{
    std::string raw =
        kJpgHeader +
        std::string(10, '\x00') +
        kTifHeaderIntel +
        std::string(10, '\xAB') +
        kJpgFooter;

    const uint64_t tifStart = kJpgHeader.size() + 10;
    const uint64_t lastIndex = raw.size() - 1;

    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 2u);

    auto jpgRange = result[0]->m_data.front().front();
    EXPECT_EQ(jpgRange.first, 0u);
    EXPECT_EQ(jpgRange.second, lastIndex);

    auto tifRange = result[1]->m_data.front().front();
    EXPECT_EQ(tifRange.first, tifStart);
    EXPECT_EQ(tifRange.second, lastIndex);
    delete carve;
}

TEST_F(CarveRuleTest, FindsAviHeader)
{
    std::string raw = kAviHeader + std::string(9, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsMpgVariantBA)
{
    std::string raw = kMpgHeaderBA + std::string(5, '\x00') + kMpgFooterB9;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsMpgVariantB3)
{
    std::string raw = kMpgHeaderB3 + std::string(5, '\x00') + kMpgFooterB7;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsFwsHeader)
{
    std::string raw = kFwsHeader + std::string(10, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsWavHeader)
{
    std::string raw = kWavHeader + std::string(8, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsDocShortHeaderNoFooter)
{
    std::string raw = kDocShortHeader + std::string("\xFF", 1) + std::string(9, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, DocNextKeywordHeaderMatchesBothDocSignatures)
{
    std::string raw = kDocNextHeader + std::string(5, '\x00') + kDocNextHeader;
    const uint64_t lastIndex = raw.size() - 1;
    const uint64_t secondHeaderOffset = kDocNextHeader.size() + 5;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 3u);

    auto range0 = result[0]->m_data.front().front();
    EXPECT_EQ(range0.first, 0u);
    EXPECT_EQ(range0.second, lastIndex);

    auto range1 = result[1]->m_data.front().front();
    EXPECT_EQ(range1.first, 0u);
    EXPECT_EQ(range1.second, lastIndex);

    auto range2 = result[2]->m_data.front().front();
    EXPECT_EQ(range2.first, secondHeaderOffset);
    EXPECT_EQ(range2.second, lastIndex);

    delete carve;
}

TEST_F(CarveRuleTest, FindsPstHeader)
{
    std::string raw = kPstHeader + std::string(10, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsOstHeader)
{
    std::string raw = kOstHeader + std::string(10, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsDbxHeader)
{
    std::string raw = kDbxHeader + std::string(10, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsIdxHeader)
{
    std::string raw = kIdxHeader + std::string(10, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsMbxHeader)
{
    std::string raw = kMbxHeader + std::string(10, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsHtmWithFooter)
{
    std::string raw = kHtmHeader + std::string(5, '\x00') + kHtmFooter;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsPdfWithCarriageReturnFooter)
{
    std::string raw = kPdfHeader + std::string(5, '\x00') + kPdfFooterCR;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsPdfWithLineFeedFooter)
{
    std::string raw = kPdfHeader + std::string(5, '\x00') + kPdfFooterLF;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsZipWithFooter)
{
    std::string raw = kZipHeader + std::string(5, '\x00') + kZipFooter;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsJavaHeader)
{
    std::string raw = kJavaHeader + std::string(10, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsTgzHeader)
{
    std::string raw = kTgzHeader + std::string(10, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, FindsOggWithSelfReferentialFooter)
{
    std::string raw = kOggPattern + std::string(5, '\x00') + kOggPattern;
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    ASSERT_EQ(result.size(), 1u);
    auto range = result[0]->m_data.front().front();
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.second, raw.size() - 1);
    delete carve;
}

TEST_F(CarveRuleTest, NoMatchProducesNoChildren)
{
    std::string raw(64, '\x00');
    AddressNodePtr node = BuildFullRangeNode(raw);

    Rule* carve = new Carve();
    AddressNodeList result = carve->evaluate({ node });

    EXPECT_TRUE(result.empty());
    delete carve;
}