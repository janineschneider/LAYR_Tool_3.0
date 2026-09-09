#ifndef EXT3_FSL_H
#define EXT3_FSL_H

#include "../../rule.h"

/**
 * \brief Ext3 file system evaluation rule
 * \author Janine Schneider
 * \author Josef Ilg
 * \author Marie Becker
 * \date May, 2026
 */
class Ext3_fsl : public Rule
{
public:
    Ext3_fsl();
    ~Ext3_fsl();

    /**
     * \brief Rule evaluation of possible block sequences representing an Ext3 file system
     * \param input Input AddressNodes whose byte source is searched for an Ext3 file system structure
     * \return AddressNodeList containing reconstructionNodes representing the found file system and its corresponding metadata
     */
    AddressNodeList evaluate(AddressNodeList input);
};
#endif // EXT3_FSL_H
