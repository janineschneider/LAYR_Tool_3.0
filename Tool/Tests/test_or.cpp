#include "gtest/gtest.h"
#include "../tree.h"
#include "../SequentialOperators/or.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class MockSuccessRule : public Rule
{
public:
    explicit MockSuccessRule(std::string tag) : m_tag(std::move(tag)) {}

    AddressNodeList evaluate(AddressNodeList input) override
    {
        AddressNodeList result;
        auto dummyNode = std::make_shared<reconstructionNode>(
            std::vector<std::vector<std::pair<uint64_t, uint64_t>>>{},
            std::vector<std::vector<std::pair<uint64_t, uint64_t>>>{},
            m_tag
        );
        result.push_back(dummyNode);
        return result;
    }

private:
    std::string m_tag;
};

class MockEmptyRule : public Rule
{
public:
    AddressNodeList evaluate(AddressNodeList input) override
    {
        return {};
    }
};

class MockFailingRule : public Rule
{
public:
    AddressNodeList evaluate(AddressNodeList input) override
    {
        throw std::runtime_error("Rule evaluation failed!");
    }
};

class MockCountingRule : public Rule
{
public:
    explicit MockCountingRule(int* counter) : m_counter(counter) {}

    AddressNodeList evaluate(AddressNodeList input) override
    {
        if (m_counter) {
            ++(*m_counter);
        }
        return {};
    }

private:
    int* m_counter;
};

class MockDestructorRule : public Rule
{
public:
    static int instanceCount;

    MockDestructorRule() { ++instanceCount; }
    ~MockDestructorRule() noexcept override { --instanceCount; }

    AddressNodeList evaluate(AddressNodeList input) override
    {
        return {};
    }
};

int MockDestructorRule::instanceCount = 0;

class OrOperatorTest : public ::testing::Test
{
protected:
    AddressNodeList emptyInput;
};

TEST_F(OrOperatorTest, ConstructorThrowsOnNullPointers)
{
    Rule* validRule = new MockSuccessRule("Success");

    EXPECT_THROW(Or(nullptr, validRule), std::invalid_argument);
    EXPECT_THROW(Or(validRule, nullptr), std::invalid_argument);
    EXPECT_THROW(Or(nullptr, nullptr), std::invalid_argument);

    delete validRule;
}

TEST_F(OrOperatorTest, ConstructorAcceptsValidRules)
{
    EXPECT_NO_THROW({
        Or op(new MockSuccessRule("A"), new MockSuccessRule("B"));
        });
}

TEST_F(OrOperatorTest, ReturnsResultFromFirstRuleWhenItSucceeds)
{
    Rule* r1 = new MockSuccessRule("Rule1");
    Rule* r2 = new MockSuccessRule("Rule2");
    Or op(r1, r2);

    AddressNodeList result = op.evaluate(emptyInput);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->m_tag, "Rule1");
}

TEST_F(OrOperatorTest, DoesNotEvaluateSecondRuleWhenFirstRuleSucceeds)
{
    int secondCalls = 0;
    Rule* r1 = new MockSuccessRule("Rule1");
    Rule* r2 = new MockCountingRule(&secondCalls);
    Or op(r1, r2);

    AddressNodeList result = op.evaluate(emptyInput);

    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->m_tag, "Rule1");
    EXPECT_EQ(secondCalls, 0);
}

TEST_F(OrOperatorTest, ReturnsEmptyResultWhenFirstRuleReturnsEmpty)
{
    int secondCalls = 0;
    Rule* r1 = new MockEmptyRule();
    Rule* r2 = new MockCountingRule(&secondCalls);
    Or op(r1, r2);

    AddressNodeList result = op.evaluate(emptyInput);

    EXPECT_TRUE(result.empty());
    EXPECT_EQ(secondCalls, 0);
}

TEST_F(OrOperatorTest, FallsBackToSecondRuleWhenFirstRuleThrows)
{
    Rule* r1 = new MockFailingRule();
    Rule* r2 = new MockSuccessRule("Rule2");
    Or op(r1, r2);

    AddressNodeList result = op.evaluate(emptyInput);

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->m_tag, "Rule2");
}

TEST_F(OrOperatorTest, PropagatesExceptionWhenBothRulesFail)
{
    Rule* r1 = new MockFailingRule();
    Rule* r2 = new MockFailingRule();
    Or op(r1, r2);

    EXPECT_THROW(op.evaluate(emptyInput), std::runtime_error);
}

TEST_F(OrOperatorTest, DeletesOwnedRulesOnDestruction)
{
    MockDestructorRule::instanceCount = 0;
    {
        Or op(new MockDestructorRule(), new MockDestructorRule());
        EXPECT_EQ(MockDestructorRule::instanceCount, 2);
    }
    EXPECT_EQ(MockDestructorRule::instanceCount, 0);
}
