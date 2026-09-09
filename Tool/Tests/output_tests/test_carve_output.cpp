#include "../../getworkingdir.h"
#include "../../Rules/Carve/carve.h"
#include "../../tree.h"
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
    imagePath.append(slash + "TestImages" + slash + "NTFS" + slash);

    //Read in image name
    std::string imageName;
    imageName = "testimage_ntfs.dd";
    imagePath.append(imageName);
    std::cout << imagePath << std::endl;

    std::cout << "[+] Scalpel test is running ..." << std::endl;

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);

    std::cout << "creating rule" << std::endl;;

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        std::cout << "creating rule" << std::endl;;

        AddressTreeRoot tree(rawData);
        AddressNodeList output;

        AddressNodeList input;
        input.push_back(tree.tree);

        std::cout << "creating rule" << std::endl;;

        Rule* r1;
        r1 = new scalpel();
        output = r1->evaluate(input);

        std::cout << "FilesFound:" << std::endl;
        for (AddressNodePtr& node : output) {
            for (auto& it : node->m_data) {
                if (it.size() > 0) {
                    for (auto& it2 : it) {
                        std::cout << "filetype:" << node->m_tag << "start: " << it2.first << " end: " << it2.second;
                    }
                }
                else {
                    std::cout << "NO DATA";
                }
                std::cout << std::endl;
            }
        }

        std::cout << "\nMetadata:" << std::endl;
        for (AddressNodePtr& node : output) {
            for (auto& it : node->m_metadata) {
                for (auto& it2 : it) {
                    std::cout << "filetype:" << node->m_tag << "start: " << it2.first << " end: " << it2.second;
                }
                std::cout << std::endl;
            }
        }

        std::cout << std::endl;

        std::cout << "===== Tree Structure =====" << std::endl;
        tree.print();
        std::cout << "===========================" << std::endl;

        imageStream.close();
    }

    return 0;
}
