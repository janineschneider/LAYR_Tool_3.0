#ifndef OUTPUTrule_H
#define OUTPUTrule_H

#include "rule.h"

/**
 * \brief Output rule base class
 * \author Janine Schneider
 * \date January, 2019
 */
class OutputRule
{
public:
    OutputRule() {};
    OutputRule(AddressNodeList rawData) : m_rawData(rawData) {};
    virtual ~OutputRule() noexcept {}

    /**
     * \brief Evaluates the given AddressNodes and prints the resulting output to cout
     * \param input Input AddressNodes to evaluate
     */
    virtual void evaluate(AddressNodeList input) = 0;

private:
    AddressNodeList m_rawData;
};

#endif // OUTPUTrule_H
