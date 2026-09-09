#ifndef NTFS_FL_DFXML_H
#define NTFS_FL_DFXML_H

/**
 * \brief NTFS files evaluation dfxml rule
 * \author Janine Schneider
 * \author Josef Ilg
 * \author Marie Becker
 * \date Mai, 2026
 */

#include "../../outputrule.h"
#include "../DFXML/dfxml_writer.h"

/**
 * \brief DFXML output for NTFS file metadata (inodes)
 * \author Janine Schneider
 * \author Josef Ilg
 * \author Marie Becker
 * \date Mai, 2026
 */
class Ntfs_fl_dfxml : public OutputRule
{
public:
    Ntfs_fl_dfxml();
    ~Ntfs_fl_dfxml();

    /**
     * \brief Serialize NTFS file metadata to DFXML and print it to standard output.
     * \param input AddressNodeList containing pointers to AddressNodes with NTFS file metadata.
     */
    void evaluate(AddressNodeList input);

private:
    /**
     * Members
     */
    std::string nanoseconds2DateTime(uint64_t nanoseconds);
    void printFileName(AddressNodePtr node, dfxml_writer* outputWriter, uint64_t attributePosition, uint64_t indexOffset, uint64_t lowerBound, uint64_t upperBound);
    void printStandardInformation(AddressNodePtr node, dfxml_writer* outputWriter, uint64_t attributePosition, uint64_t indexOffset, uint64_t lowerBound, uint64_t upperBound);
};

#endif // NTFS_FL_DFXML_H
