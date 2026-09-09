#pragma once

#include "gtest/gtest.h"
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../../tree.h"

class TreeTestFixture : public ::testing::Test
{
protected:
    std::unique_ptr<std::istringstream> input_stream;
    std::unique_ptr<ByteContainer> input_data;
    std::unique_ptr<AddressTreeRoot> tree;

    void SetUp() override;
    void TearDown() override;

    void CreateTestTree(const std::string& raw = "12345");
    AddressNodePtr MakeReconstructionNode(
        const std::string& tag,
        const std::vector<std::vector<std::pair<uint64_t, uint64_t>>>& data) const;
    AddressNodePtr MakeTransformationNode(
        const std::string& tag,
        const std::vector<std::vector<std::pair<uint64_t, uint64_t>>>& data) const;

    void AddLayer1();
    void AddLayer2();
    void AddLayer3();

    std::vector<AddressNode*> CollectNodes(AddressNode* start = nullptr) const;
    uint64_t ComputeMaxDepth(AddressNode* start = nullptr) const;
    void AssertTreeIntegrity() const;
    void AssertLayerCount(uint64_t level, size_t expected) const;

private:
    void AssertChildConsistency(AddressNode* node) const;
};
