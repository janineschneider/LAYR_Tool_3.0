/**
 * @file fat32_fl.h
 * @brief Reconstruction rule for the FAT32 file layer
 */
#ifndef FAT32_FL_H
#define FAT32_FL_H


#include "../../rule.h"
#include <vector>

class FAT32_fl : public Rule
{
public:
    FAT32_fl();
    ~FAT32_fl();

    /**
     * \brief Rule evaluation of possible FAT32 block sequences representing files and directories
     * \param input Input AddressNodes whose byte source is searched for FAT32 file structures
     * \return AddressNodeList containing reconstructionNodes representing found files/directories and their corresponding metadata
     */
    AddressNodeList evaluate(AddressNodeList input);

    /**
     * \brief Evaluates FAT cluster chains to determine which clusters belong to the same file
     * \param fatRange Ranges of raw FAT table entries to evaluate
     * \return Vector of cluster chains, each entry containing a cluster number and the next cluster in the chain
     */
    std::vector<std::vector<std::pair<uint32_t, int32_t>>> evaluateFAT(std::vector<std::vector<uint32_t>> fatRange);

private:
    std::string tag = "FAT32_fl";
    ByteContainer* m_rawData;
    //Result vector
    std::vector<std::pair<uint64_t, uint64_t>> dataSeqence;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> dataSeqences;
    std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;
    AddressNodeList output;
    std::pair<uint64_t, uint64_t> indexPair;

    //FAT32 attributes
    uint32_t rootDirectoryStart;
    uint64_t startCluster;
    uint32_t clusterSize;
    uint16_t sectorSize;
    std::vector<std::vector<std::pair<uint32_t, int32_t>>> fatEntries;
    //Input Attribute
    AddressNodePtr input;


    //Helper functions
    std::string evaluateAttributes(unsigned char hex);
    void handleDirectories();
    void addDataPair(uint64_t sector);
    uint32_t cluster2Sector(uint32_t cluster, std::vector<std::vector<uint32_t>> fatRange);
};

#endif // FAT32_FL_H
