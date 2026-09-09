#ifndef UNION_H
#define UNION_H

#include "../parallel.h"

/**
 * \brief Union operator
 * \author Janine Schneider
 * \author Marie Becker
 * \date August, 2026
 */
class Union : public Parallel
{
public:
    Union(Rule* r1, Rule* r2) : Parallel(r1, r2) {}

    /**
     * \brief Union operator function \n
     *        Combines the results of two rules by joining them together
     * \param res1 The results of r1
     * \param res2 The results of r2
     * \return Returns the union of two rule results
     */
    AddressNodeList setOperator(AddressNodeList res1, AddressNodeList res2);
};

#endif // UNION_H
