#ifndef MINUS_H
#define MINUS_H

#include "../parallel.h"

/**
 * \brief Minus operator
 * \author Janine Schneider
 * \date January, 2019
 * \author Marie Becker
 * \date August, 2026
 */
class Minus : public Parallel
{
public:
    Minus(Rule* r1, Rule* r2) : Parallel(r1, r2) {}

    /**
     * \brief Minus operator function \n
     *        Removes the results of the r2 from the results of r1
     * \param res1 AddressNodeList returned by r1
     * \param res2 AddressNodeList returned by r2
     * \return Returns an AddressNodeList containing the result of r1, without the results of the rule evaluation r2
     */
    AddressNodeList setOperator(AddressNodeList res1, AddressNodeList res2);
};

#endif // MINUS_H
