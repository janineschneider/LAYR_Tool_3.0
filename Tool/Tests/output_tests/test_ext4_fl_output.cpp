#include "../../getworkingdir.h"
#include "../../Rules/Ext4/ext4_fsl.h"
#include "../../Rules/Ext4/ext4_fl.h"

#include <fstream>

int main()
{
    // Get current working dir path
    // Currently working for Windows
    // Please change define in getworkingdir.h for Unix
    char buffer[10000];
    getCurrentDir(buffer, 1000);
    std::string imagePath(buffer);

    // Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    std::string imagePath2(imagePath);
    imagePath.append(slash + "TestImages" + slash + "EXT4" + slash);
    imagePath2.append(slash + "buildDebug32" + slash);

    // Read in image name
    std::string imageName;
    imageName = "test_ext4.dd";
    imagePath.append(imageName);

    // Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList output;
        AddressNodeList output2;

        Rule* r1;
        r1 = new Ext4_fsl();
        AddressNodeList input;
        input.push_back(tree.tree);
        output = r1->evaluate(input);

        Rule* r2;
        r2 = new Ext4_fl();
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

        // Graphviz graph creation
        if (tree.tree) {
            try {
                tree.tree->save_as_dot("tree.dot");
                std::cout << "\n[Graphviz] Graph successfully exported to tree.dot" << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "[DOT Export Error] " << e.what() << std::endl;
            }
        }

        imageStream.close();
    }

    return 0;
}