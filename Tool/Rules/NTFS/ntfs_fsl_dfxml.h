#ifndef NTFS_FSL_DFXML_H
#define NTFS_FSL_DFXML_H

#include "../../outputrule.h"

/**
 * \brief DFXML output for NTFS file system metadata
 * \author Janine Schneider
 * \author Josef Ilg
 * \author Marie Becker
 * \date Mai, 2026
 */
class Ntfs_fsl_dfxml : public OutputRule
{
public:
    Ntfs_fsl_dfxml();
    ~Ntfs_fsl_dfxml();

    /**
     * \brief Serialize NTFS file system metadata to DFXML and print it to standard output.
     * \param input AddressNodeList containing pointers to AddressNodes with NTFS file system metadata.
     */
    void evaluate(AddressNodeList input);
};

#endif // NTFS_FSL_DFXML_H
