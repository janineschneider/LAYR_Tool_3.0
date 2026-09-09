#include "seqcomp.h"
#include "../tree.h"
#include <algorithm>

SeqCompMultiChoice::SeqCompMultiChoice(Rule* h1, Rule* h2, Choose chooser) : m_h1(h1), m_h2(h2), m_chooser(chooser) {}

SeqCompMultiChoice::~SeqCompMultiChoice()
{
    if (m_h1 != nullptr) {
        delete m_h1;
    }

    if (m_h2 != nullptr) {
        delete m_h2;
    }
}

AddressNodeList SeqCompMultiChoice::evaluate(AddressNodeList input)
{
    AddressNodeList h1_res = m_h1->evaluate(input);
    AddressNodeList choice = m_chooser(h1_res);
    AddressNodeList h2_res = m_h2->evaluate(choice);

    return h2_res;
}

SeqCompMulti::SeqCompMulti(Rule* h1, Rule* h2, Choose chooser) : m_h1(h1), m_h2(h2), m_chooser(chooser) {}

SeqCompMulti::~SeqCompMulti()
{
    if (m_h1 != nullptr) {
        delete m_h1;
    }

    if (m_h2 != nullptr) {
        delete m_h2;
    }
}

AddressNodeList SeqCompMulti::evaluate(AddressNodeList input)
{
    AddressNodeList h1_res = m_h1->evaluate(input);
    AddressNodeList h2_res = m_h2->evaluate(h1_res);

    return h2_res;
}

SeqCompSingle::SeqCompSingle(Rule* h1, Rule* h2) : m_h1(h1), m_h2(h2) {}

SeqCompSingle::~SeqCompSingle()
{
    if (m_h1 != nullptr) {
        delete m_h1;
    }

    if (m_h2 != nullptr) {
        delete m_h2;
    }
}

AddressNodeList SeqCompSingle::evaluate(AddressNodeList input)
{
    AddressNodeList h1_res = m_h1->evaluate(input);
    AddressNodeList h2_res;
    for (AddressNodePtr node : h1_res) {
        AddressNodeList temp = m_h2->evaluate(std::vector{ node });
        h2_res.insert(h2_res.end(), temp.begin(), temp.end());
    }
    return h2_res;
}

SeqCompSingleChoice::SeqCompSingleChoice(Rule* h1, Rule* h2, Choose chooser) : m_h1(h1), m_h2(h2), m_chooser(chooser) {}

SeqCompSingleChoice::~SeqCompSingleChoice()
{
    if (m_h1 != nullptr) {
        delete m_h1;
    }

    if (m_h2 != nullptr) {
        delete m_h2;
    }
}

AddressNodeList SeqCompSingleChoice::evaluate(AddressNodeList input)
{
    AddressNodeList h1_res = m_chooser(m_h1->evaluate(std::vector{ input.at(0) }));
    AddressNodeList h2_res;
    for (AddressNodePtr node : h1_res) {
        AddressNodeList temp = m_h2->evaluate(std::vector{ node });
        h2_res.insert(h2_res.end(), temp.begin(), temp.end());
    }
    return h2_res;
}