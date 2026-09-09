#include "../../getworkingdir.h"
#include "../../tree.h"
#include "../../Rules/NTFS/ntfs_fsl.h"
#include "../../Rules/NTFS/ntfs_fsl_dfxml.h"
#include <fstream>
#include <iostream>


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

    std::cout << "searching for testimage in path: " << imagePath << std::endl;

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        std::vector<std::pair<uint64_t, uint64_t>> helper;
        helper.push_back(std::make_pair(0, rawData->size()));
        AddressNodeList output;

        Rule* r1;
        r1 = new Ntfs_fsl();
        AddressNodeList input;
        input.push_back(tree.tree);
        output = r1->evaluate(input);
        tree.print();


        std::cout << "Contentdata:" << std::endl;
        for (AddressNodePtr& node : output) {
            for (auto& it : node->m_data) {
                for (auto& it2 : it) {
                    std::cout << "start: " << it2.first << " end: " << it2.second;
                }
                std::cout << std::endl;
            }
        }


        std::cout << "\nMetadata:" << std::endl;
        for (AddressNodePtr& node : output) {
            for (auto& it : node->m_metadata) {
                for (auto& it2 : it) {
                    std::cout << "start: " << it2.first << " end: " << it2.second;
                }
                std::cout << std::endl;
            }
        }

        std::cout << std::endl;

        OutputRule* or1;
        or1 = new Ntfs_fsl_dfxml();
        or1->evaluate(output);


        imageStream.close();
    }

    return 0;
}
