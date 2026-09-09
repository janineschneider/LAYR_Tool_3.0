#include "or.h"
#include <exception>

Or::Or(Rule* r1, Rule* r2) : m_r1(r1), m_r2(r2)
{
    if (!r1 || !r2) {
        throw std::invalid_argument("Or rule requires two non-null Rule pointers.");
    }
}

Or::~Or()
{
    if (m_r1 != nullptr) {
        delete m_r1;
    }

    if (m_r2 != nullptr) {
        delete m_r2;
    }
}

AddressNodeList Or::evaluate(AddressNodeList input)
{
    AddressNodeList res;
    try {
        res = m_r1->evaluate(input);
    }
    catch (std::exception& e) {
        res = m_r2->evaluate(input);
    }

    return res;
}
