#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "../parallel.h"

/**
 * \brief Intersection operator
 * \author Janine Schneider
 * \author Marie Becker
 * \date July, 2026
 */
class Intersection : public Parallel
{
public:
    Intersection(Rule* r1, Rule* r2) : Parallel(r1, r2) {}

    /**
     * \brief Intersection operator function \n
     *        Matches every node in res1 against every node in res2 by content
     *        data (m_data), metadata is ignored for matching
     * \param res1 AddressNodeList containing all results of r1
     * \param res2 AddressNodeList containing all results of r2
     * \return Returns an AddressNodeList containing the intersection of two rule results
     */
    AddressNodeList setOperator(AddressNodeList res1, AddressNodeList res2);
};

#endif // INTERSECTION_H
