#ifndef OR_H
#define OR_H

#include "../rule.h"

/**
 * \file or.h
 * \brief Or operator
 */
class Or : public Rule
{
public:
    Or(Rule* r1, Rule* r2);
    ~Or();

    /**
     * \brief Or operator function \n
     *        Runs r2 if r1 fails
     * \param input Input AddressNodes to be evaluated
     * \return AddressNodeList containing the created nodes
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    Rule* m_r1;
    Rule* m_r2;
};

#endif // OR_H
