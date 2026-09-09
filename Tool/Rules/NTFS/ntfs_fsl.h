#ifndef NTFS_FSL_H
#define NTFS_FSL_H

#include "../../rule.h"

/**
 * \brief NTFS file system evaluation rule
 * \author Janine Schneider
 * \author Josef Ilg
 * \author Marie Becker
 * \date Mai, 2026
 */
class Ntfs_fsl : public Rule
{
public:
    Ntfs_fsl();
    ~Ntfs_fsl();

    /**
     * \brief Rule evaluation of an NTFS file system
     * \param input AddressNodeList containing pointers to AddressNodes for raw NTFS file system blocks.
     * \return AddressNodeList with pointers to reconstructionNodes containing file system content and metadata.
     *
     *         contentData = [data area]
     *         metaData    = [[boot sector], [MFT table]]
     */
    AddressNodeList evaluate(AddressNodeList input);
};

#endif // NTFS_FSL_H
