#ifndef FAT32_FSL_DFXML_H
#define FAT32_FSL_DFXML_H


#include "../../rule.h"

/**
 * \brief DFXML output for Fat32 file system metadata
 */
class Fat32_fsl_dfxml : public Rule
{
public:
    Fat32_fsl_dfxml();
    ~Fat32_fsl_dfxml();

    /**
     * \brief DFXML output for Fat32 file system metadata
     *        Prints the output to cout
     * \param input Input nodes with contentdata and metadata as block sequences
     */
    AddressNodeList evaluate(AddressNodeList input);
};


#endif // FAT32_FSL_DFXML_H