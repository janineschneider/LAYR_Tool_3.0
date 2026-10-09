#ifndef EXT3_FL_DFXML_H
#define EXT3_FL_DFXML_H

#include "../../outputrule.h"

/**
 * \brief DFXML output for Ext3 file metadata (inodes)
 */
class Ext3_fl_dfxml : public OutputRule
{
public:
    Ext3_fl_dfxml();
    ~Ext3_fl_dfxml();

    /**
     * \brief DFXML output for Ext3 file metadata (inodes)
     *        Prints the output to cout
     * \param input Input AddressNodes whose m_data ranges and metadata represent Ext3 file inodes
     */
    void evaluate(AddressNodeList input);

private:
    /**
     * \brief Converst unix timestamp to human readable date and time.
     * \param Unix timestamp
     * \return Date and time
     */
    std::string unix2DateTime(uint32_t u);

    /**
     * \brief Converts feature flags to human readable features.
     * \param Two bytes as unsigned char array
     * \return The human readable feature string
     */
    std::string evaluateROCompatFeatures(unsigned char hex[]);
};

#endif // EXT3_FL_DFXML_H
