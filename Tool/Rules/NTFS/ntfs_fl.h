#ifndef NTFS_FL_H
#define NTFS_FL_H

#include "../../rule.h"
#include <string>
/**
 * \brief NTFS files evaluation rule
 */
class Ntfs_fl : public Rule
{
public:
    Ntfs_fl();
    ~Ntfs_fl();

    /**
     * \brief Rule evaluation of NTFS file system
     * \param input AddressNodeList containing pointers to AddressNodes with NTFS file system blocks.
     * \return AddressNodeList with pointers to reconstructionNodes containing file content and metadata.
     *
     *         contentData = [data of files]
     *         metaData    = [MFT entry of file]
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    //Result vector
    std::vector<std::pair<uint64_t, uint64_t>> contentSeqence;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> contentSeqences;
    std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
    std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence2;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
    AddressNodeList new_children;
    AddressNodeList output;
    std::string tag = "ntfs_fl";

    bool isMftEmpty(std::vector<unsigned char>* mftData, uint64_t position);
    std::string getFileName(std::vector<unsigned char>* mftData, uint64_t attributePosition);

};

#endif // NTFS_FL_H



