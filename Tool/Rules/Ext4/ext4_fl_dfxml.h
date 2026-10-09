#ifndef EXT4_FL_DFXML_H
#define EXT4_FL_DFXML_H

#include "../../outputrule.h"

/**
 * \brief DFXML output for Ext4 file metadata (inodes)
 */
class Ext4_fl_dfxml : public OutputRule
{
public:
    Ext4_fl_dfxml();
    ~Ext4_fl_dfxml();

    /**
     * \brief DFXML output for Ext4 file metadata (inodes)
     *        Prints the output to cout
     * \param input Input AddressNodes created by Ext4_fl: m_data = block runs of the file,
     *              m_metadata = location of the inode record
     */
    void evaluate(AddressNodeList input);

private:
    /**
     * \brief Converts unix timestamp to human readable date and time.
     * \param u Unix timestamp
     * \return Date and time
     */
    std::string unix2DateTime(uint32_t u);

    /**
     * \brief Reads a little-endian integer from the raw data
     */
    uint64_t readLE(AddressNodePtr& node, uint64_t offset, uint64_t position, int bytes,
        uint64_t lowerBound, uint64_t upperBound);
};

#endif // EXT4_FL_DFXML_H