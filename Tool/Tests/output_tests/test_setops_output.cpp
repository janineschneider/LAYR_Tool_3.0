#include "../../getworkingdir.h"
#include "../../tree.h"
#include "../../Rules/Ext3/ext3_fsl.h"
#include "../../Rules/Ext3/ext3_fl.h"
#include "../../ParallelOperators/union.h"
#include "../../ParallelOperators/intersection.h"
#include "../../ParallelOperators/minus.h"

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
    imagePath.append(slash + "TestImages" + slash + "EXT3" + slash);
    imagePath2.append(slash + "buildDebug" + slash);

    //Read in image name
    std::string imageName;
    imageName = "ext3-img-kw-1.dd";
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
        r1 = new Ext3_fsl();
        output = r1->evaluate(input);

        Rule* r2;
        r2 = new Ext3_fl();

        Rule* r3;
        r3 = new Ext3_fl();

        Parallel* op;
        op = new Union(r2, r3);
        output2 = op->evaluate(output);

        std::cout << "Union Contentdata:" << std::endl;
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

        delete op;

        op = new Intersection(r2, r3);
        output2 = op->evaluate(output);

        std::cout << "Intersection Contentdata:" << std::endl;
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

        std::cout << std::endl;

        delete op;

        op = new Minus(r2, r3);
        output2 = op->evaluate(output);

        std::cout << "Minus Contentdata:" << std::endl;
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

        delete op;
        imageStream.close();
    }

    return 0;
}