#ifndef DOS_DFXML_H
#define DOS_DFXML_H

#include "../../outputrule.h"

/**
 * \brief DFXML output for DOS partition metadata
 */
class DOS_dfxml : public OutputRule
{
public:
    DOS_dfxml();
    ~DOS_dfxml();

    /**
     * \brief DFXML output for DOS partition metadata
     *        Prints the output to cout
     * \param input Input AddressNodes whose metadata sequences represent DOS partition table entries
     */
    void evaluate(AddressNodeList input);

private:
    /**
     * \brief Reads a little-endian integer from the raw data
     * \param node Node providing access to the raw data
     * \param offset Base offset
     * \param position Position relative to offset
     * \param bytes Number of bytes (1-8)
     * \param lowerBound Lower access bound
     * \param upperBound Upper access bound
     */
    uint64_t readLE(AddressNodePtr& node, uint64_t offset, uint64_t position, int bytes,
        uint64_t lowerBound, uint64_t upperBound);
};

#endif // DOS_DFXML_H