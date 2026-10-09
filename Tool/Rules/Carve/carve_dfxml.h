#ifndef CARVE_DFXML_H
#define CARVE_DFXML_H

#include "../../outputrule.h"

/**
 * \brief DFXML output for carved files
 */
class Carve_dfxml : public OutputRule
{
public:
    Carve_dfxml();
    ~Carve_dfxml();

    /**
     * \brief DFXML output for carved files
     *        Prints the output to cout
     * \param input Input AddressNodes created by Carve (tag "carve_<type>"), m_data = carved byte range
     */
    void evaluate(AddressNodeList input);
};

#endif // CARVE_DFXML_H