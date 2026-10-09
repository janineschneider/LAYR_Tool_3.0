/**
 * @file seqcomp.h
 * @brief Sequential Composition Operators
 */
#ifndef SEQCOMP_H
#define SEQCOMP_H

#include "../rule.h"
#include "../tree.h"
#include <functional>

 // Typedef of choose function
using Choose = std::function<AddressNodeList(AddressNodeList)>;

/**
 * @brief seqcomp operator with single input and without choice
 */
class SeqCompSingle : public Rule
{
public:
    SeqCompSingle(Rule* h1, Rule* h2);
    ~SeqCompSingle();

    /**
     * \brief Sequential composition operator function \n
     *        Evaluates h1, evaluates h2 on all resulting AddressNodes, combines the results
     * \param input Input AddressNodes to be evaluated by h1
     * \return AddressNodeList containing the created nodes
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    Rule* m_h1;
    Rule* m_h2;
};

/**
 * @brief seqcomp operator with single input and choice
 *
 */
class SeqCompSingleChoice : public Rule
{
public:
    SeqCompSingleChoice(Rule* h1, Rule* h2, Choose chooser);
    ~SeqCompSingleChoice();

    /**
     * \brief Sequential composition operator function \n
     *        Evaluates h1, chooses a single AddressNode from the result, evaluates h2 on it, combines the results
     * \param input Input AddressNodes to be evaluated by h1
     * \return AddressNodeList containing the created nodes
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    Rule* m_h1;
    Rule* m_h2;
    Choose m_chooser;
};

/**
 * @brief SeqComp operator with multi input and without choice
 */
class SeqCompMulti : public Rule
{
public:
    SeqCompMulti(Rule* h1, Rule* h2, Choose chooser);
    ~SeqCompMulti();

    /**
     * \brief Sequential composition operator function \n
     *        Evaluates h1, evaluates h2 on all resulting AddressNodes selected by the chooser, combines the results
     * \param input Input AddressNodes to be evaluated by h1
     * \return AddressNodeList containing the created nodes
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    Rule* m_h1;
    Rule* m_h2;
    Choose m_chooser;
};

/**
 * @brief SeqComp operator with multi input and choice
 */
class SeqCompMultiChoice : public Rule
{
public:
    SeqCompMultiChoice(Rule* h1, Rule* h2, Choose chooser);
    ~SeqCompMultiChoice();

    /**
     * \brief Sequential composition operator function \n
     *        Evaluates h1, chooses a subset of the result, evaluates h2 on all AddressNodes from the chosen subset, combines the results
     * \param input Input AddressNodes to be evaluated by h1
     * \return AddressNodeList containing the created nodes
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    Rule* m_h1;
    Rule* m_h2;
    Choose m_chooser;
};

#endif // SEQCOMP_H
