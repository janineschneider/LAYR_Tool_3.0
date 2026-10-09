#ifndef EXT4_FSL_DFXML_H
#define EXT4_FSL_DFXML_H

#include "../../outputrule.h"

/**
 * \brief DFXML output for Ext4 file system metadata
 */
class Ext4_fsl_dfxml : public OutputRule
{
public:
    Ext4_fsl_dfxml();
    ~Ext4_fsl_dfxml();

    /**
     * \brief DFXML output for Ext4 file system metadata
     *        Prints the output to cout
     * \param input Input AddressNodes created by Ext4_fsl: m_data = file system range,
     *              m_metadata = superblock followed by the inode tables
     */
    void evaluate(AddressNodeList input);

private:
    /**
     * \brief Reads a little-endian integer from the raw data
     */
    uint64_t readLE(AddressNodePtr& node, uint64_t offset, uint64_t position, int bytes,
        uint64_t lowerBound, uint64_t upperBound);
};

#endif // EXT4_FSL_DFXML_H