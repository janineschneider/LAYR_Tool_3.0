#ifndef EXT3_FSL_DFXML_H
#define EXT3_FSL_DFXML_H

#include "../../outputrule.h"

/**
 * \brief DFXML output for Ext3 file system metadata
 */
class Ext3_fsl_dfxml : public OutputRule
{
public:
    Ext3_fsl_dfxml();
    ~Ext3_fsl_dfxml();

    /**
     * \brief DFXML output for Ext3 file system metadata
     *        Prints the output to cout
     * \param input Input AddressNodes whose m_data ranges and metadata represent an Ext3 file system
     */
    void evaluate(AddressNodeList input);
};

#endif // EXT3_FSL_DFXML_H
