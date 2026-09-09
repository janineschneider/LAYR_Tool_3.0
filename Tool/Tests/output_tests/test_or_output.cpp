#include "../../getworkingdir.h"
#include "../../tree.h"
#include "../../Rules/DOS/dos.h"

#include <iostream>
#include <fstream>

int main()
{
    // Get current working dir path
    // Currently working for Windows
    // Please change define in getworkingdir.h for Unix
    char buffer[100];
    getCurrentDir(buffer, 100);
    std::string imagePath(buffer);

    // Handle file extension
    size_t found = imagePath.rfind(slash);
    imagePath.erase(found, imagePath.size() - found);
    std::string imagePath2(imagePath);
    imagePath.append(slash + "TestImages" + slash + "DOS" + slash);
    imagePath2.append(slash + "buildDebug" + slash);

    // Read in image name
    std::string imageName;
    imageName = "ext-part-test-2.dd";
    imagePath.append(imageName);

    // Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        std::vector<std::pair<uint64_t, uint64_t>> helper;
        helper.push_back(std::make_pair(0, rawData->size()));
        AddressNodeList output;

        AddressNodeList input;
        input.push_back(tree.tree);

        Rule* r2;
        r2 = new DOS();

        output = r2->evaluate(input);

        for (AddressNodePtr& node : output) {
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

        imageStream.close();
    }

    return 0;
}