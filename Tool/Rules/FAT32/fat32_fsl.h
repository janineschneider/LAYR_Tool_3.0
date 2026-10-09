#ifndef FAT32_H
#define FAT32_H

#include "../../rule.h"
#include <vector>


/**
 * \brief DFXML output for Fat32 file system metadata
 */
class FAT32_fsl : public Rule
{
public:
    FAT32_fsl();
    ~FAT32_fsl();

    /**
     * \brief Rule evaluation of possible block sequences representing a FAT32 file system
     * \param input Input AddressNodes whose byte source is searched for a FAT32 file system structure
     * \return AddressNodeList containing reconstructionNodes representing the found file system and its corresponding metadata
     */
    AddressNodeList evaluate(AddressNodeList input);
};

#endif // FAT32_H
