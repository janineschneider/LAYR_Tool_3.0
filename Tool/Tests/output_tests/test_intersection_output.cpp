#include "../../getworkingdir.h"
#include "../../tree.h"
#include "../../Rules/Ext3/ext3_fsl.h"
#include "../../Rules/Ext3/ext3_fl.h"
#include "../../Rules/Ext4/ext4_fsl.h"
#include "../../Rules/Ext4/ext4_fl.h"
#include "../../SequentialOperators/seqcomp.h"
#include "../../ParallelOperators/intersection.h"

#include <iostream>
#include <fstream>

AddressNodeList choose(AddressNodeList input)
{
    AddressNodeList result;
    result.push_back(input.at(0));
    return result;
}

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
    imagePath.append(slash + "TestImages" + slash + "EXT3" + slash);

    // Read in image name
    std::string imageName;
    imageName = "ext3-img-kw-1.dd";   // adjust if you want an image containing both fs types
    imagePath.append(imageName);

    // Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList input;
        input.push_back(tree.tree);
        AddressNodeList output;

        Rule* ext3_fsl = new Ext3_fsl();
        Rule* ext3_fl = new Ext3_fl();
        AddressNodeList ext3_mid = ext3_fsl->evaluate(input);
        AddressNodeList ext3_result = ext3_fl->evaluate(ext3_mid);

        /*
        std::cout << "Ext3 results:" << std::endl;
        for (AddressNodePtr& node : ext3_result) {
            for (auto& it : node->m_data) {
                if (it.size() > 0) {
                    for (auto& it2 : it) {
                        std::cout << "start: " << it2.first << " end: " << it2.second << std::endl;
                    }
                }
                else {
                    std::cout << "NO DATA" << std::endl;
                }
            }
        }
        */

        Rule* ext4_fsl = new Ext4_fsl();
        Rule* ext4_fl = new Ext4_fl();
        AddressNodeList ext4_mid = ext4_fsl->evaluate(input);
        AddressNodeList ext4_result = ext4_fl->evaluate(ext4_mid);

        /*
        std::cout << "Ext4 results:" << std::endl;
        for (AddressNodePtr& node : ext4_result) {
            for (auto& it : node->m_data) {
                if (it.size() > 0) {
                    for (auto& it2 : it) {
                        std::cout << "start: " << it2.first << " end: " << it2.second << std::endl;
                    }
                }
                else {
                    std::cout << "NO DATA" << std::endl;
                }
            }
        }
        */

        // P = r1 . r2  (Intersection)
        Intersection op(nullptr, nullptr); // setOperator called directly
        output = op.setOperator(ext3_result, ext4_result);

        std::cout << "Intersection (Ext3 . Ext4) results:" << std::endl;
        for (AddressNodePtr& node : output) {
            for (auto& it : node->m_data) {
                if (it.size() > 0) {
                    for (auto& it2 : it) {
                        std::cout << "start: " << it2.first << " end: " << it2.second << std::endl;
                    }
                }
                else {
                    std::cout << "NO DATA" << std::endl;
                }
            }
        }

        imageStream.close();
    }

    return 0;
}