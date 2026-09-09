#include "../../getworkingdir.h"
#include "../../Rules/Ext4/ext4_fsl.h"
#include <fstream>

int main()
{
    // Get current working dir path
    char buffer[1000];
    getCurrentDir(buffer, 1000);
    std::string imagePath(buffer);

    // Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    std::string imagePath2(imagePath);
    imagePath.append(slash + "TestImages" + slash + "EXT4" + slash);

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
        std::vector<std::pair<uint64_t, uint64_t>> helper;
        helper.push_back(std::make_pair(0, rawData->size()));
        AddressNodeList input;
        input.push_back(tree.tree);
        AddressNodeList output;

        Ext4_fsl* r1 = new Ext4_fsl();
        output = r1->evaluate(input);

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