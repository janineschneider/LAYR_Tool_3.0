#include "../../getworkingdir.h"
#include "../../tree.h"
#include "../../Rules/FAT32/fat32_fsl.h"
#include "../../Rules/FAT32/fat32_fl.h"
#include "../../Rules/FAT32/fat32_fl_dfxml.h"

#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    //Get current working dir path
    //Currently working for Windows
    //Please change define in getworkingdir.h for Unix
    char buffer[100];
    getCurrentDir(buffer, 100);
    string imagePath(buffer);

    //Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    imagePath.append(slash + "TestImages" + slash + "FAT32" + slash);

    //Read in image name
    string imageName;
    imageName = "testimage_fat32.dd";
    imagePath.append(imageName);

    //Open binary ifstream
    ifstream imageStream(imagePath, ios::binary);

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        std::vector<std::pair<uint64_t, uint64_t>> helper;
        helper.push_back(std::make_pair(0, rawData->size()));
        AddressNodeList output;
        AddressNodeList output2;
        AddressNodeList input;

        Rule* r1;
        r1 = new FAT32_fsl();
        input.push_back(tree.tree);
        output = r1->evaluate(input);

        Rule* r2;
        r2 = new FAT32_fl();
        output2 = r2->evaluate(output);

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

        std::cout << std::endl;

        imageStream.close();
    }

    imageStream.close();

    return 0;
}
