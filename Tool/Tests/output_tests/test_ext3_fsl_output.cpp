#include "../../getworkingdir.h"
#include "../../Rules/Ext3/ext3_fsl.h"
#include "../../Rules/Ext3/ext3_fsl_dfxml.h"
#include <fstream>
#include <iostream>
#include <string>

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
    imagePath.append(slash + "TestImages" + slash + "EXT3" + slash);
    imagePath2.append(slash + "buildDebug32" + slash);

    //Read in image name
    std::string imageName;
    imageName = "ext3-img-kw-1.dd";
    imagePath.append(imageName);

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);

    if (imageStream.is_open()) {
        std::cout << "image stream opened" << std::endl;
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList output;

        Rule* h1;
        h1 = new Ext3_fsl();
        AddressNodeList in_nodes;
        in_nodes.push_back(tree.tree);
        output = h1->evaluate(in_nodes);
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
        or1 = new Ext3_fsl_dfxml();
        or1->evaluate(output);


        imageStream.close();
    }

    return 0;
}
