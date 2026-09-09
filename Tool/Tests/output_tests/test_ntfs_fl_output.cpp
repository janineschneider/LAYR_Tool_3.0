#include "../../getworkingdir.h"
#include "../../Rules/NTFS/ntfs_fsl.h"
#include "../../Rules/NTFS/ntfs_fl.h"
#include "../../Rules/NTFS/ntfs_fsl_dfxml.h"
#include "../../Rules/NTFS/ntfs_fl_dfxml.h"

#include <iostream>
#include <fstream>

int main()
{
    //Get current working dir path
    //Currently working for Windows
    //Please change define in getworkingdir.h for Unix
    char buffer[100];
    getCurrentDir(buffer, 100);
    std::string imagePath(buffer);

    //Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    std::string imagePath2(imagePath);
    imagePath.append(slash + "TestImages" + slash + "NTFS" + slash);
    imagePath2.append(slash + "buildDebug32" + slash);

    //Read in image name
    std::string imageName;
    imageName = "testimage_ntfs.dd";
    imagePath.append(imageName);

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        std::vector<std::pair<uint64_t, uint64_t>> helper;
        helper.push_back(std::make_pair(0, rawData->size()));
        AddressNodeList output;
        AddressNodeList output2;

        AddressNodeList input;
        input.push_back(tree.tree);

        Rule* r1;
        r1 = new Ntfs_fsl();
        output = r1->evaluate(input);

        Rule* r2;
        r2 = new Ntfs_fl();
        output2 = r2->evaluate(output);

        /*
        std::cout << "Contentdata:" << std::endl;
        for (AddressNodePtr& node : output2) {
            for (auto& it : node->m_data) {
                if (it.size() > 0) {
                    for (auto& it2 : it) {
                        std::cout << "start: " << it2.first << " end: " << it2.second;
                    }
                }
                else {
                    std::cout << "NO DATA";
                }
                std::cout << std::endl;
            }
        }

        std::cout << "\nMetadata:" << std::endl;
        for (AddressNodePtr& node : output2) {
            for (auto& it : node->m_metadata) {
                for (auto& it2 : it) {
                    std::cout << "start: " << it2.first << " end: " << it2.second;
                }
                std::cout << std::endl;
            }
        }
        */

        const uint64_t CLUSTER_SIZE = 4096;
        const uint64_t MFT_ENTRY_SIZE = 1024;
        const uint64_t MFT_START_BYTE = 696946688; // 170153 clusters * 4096 bytes

        std::cout << "=== NTFS Forensic Parser & Sleuth Kit Comparison ===" << std::endl;

        for (size_t i = 0; i < output2.size(); ++i) {
            const auto& node = output2[i];

            std::cout << "\nNode Index: " << i << std::endl;

            // Print Metadata MFT Entries
            std::cout << "  [Metadata]" << std::endl;
            for (const auto& outer : node->m_metadata) {
                for (const auto& range : outer) {
                    uint64_t mftEntryNum = (range.first - MFT_START_BYTE) / MFT_ENTRY_SIZE;
                    std::cout << "    MFT Entry " << mftEntryNum
                        << " | Bytes: " << range.first << " - " << range.second << std::endl;
                }
            }

            // Print Content Data (Distinguishing Resident vs Non-Resident)
            std::cout << "  [Content Data]" << std::endl;
            for (const auto& outer : node->m_data) {
                if (!outer.empty()) {
                    for (const auto& range : outer) {
                        // If the byte range falls within the MFT region, it's resident data embedded in the record
                        bool isResident = (range.first >= MFT_START_BYTE && range.second < (MFT_START_BYTE + (100000 * MFT_ENTRY_SIZE)));

                        if (isResident) {
                            uint64_t size = range.second - range.first + 1;
                            std::cout << "    [Resident Attribute] Bytes: " << range.first << " - " << range.second
                                << " (Size: " << size << " bytes)" << std::endl;
                        }
                        else {
                            uint64_t startCluster = range.first / CLUSTER_SIZE;
                            uint64_t endCluster = range.second / CLUSTER_SIZE;
                            std::cout << "    [Non-Resident Runlist] Clusters " << startCluster << " - " << endCluster
                                << " | Bytes: " << range.first << " - " << range.second << std::endl;
                        }
                    }
                }
                else {
                    std::cout << "    NO DATA" << std::endl;
                }
            }
            std::cout << "  [Tag] " << node->m_tag << std::endl;
            std::cout << std::endl;
        }
        /*
        OutputRule* or1;
        or1 = new Ntfs_fl_dfxml();
        or1->evaluate(output2);
        */

        imageStream.close();
    }

    return 0;
}
