#include "../../getworkingdir.h"
#include "../../Rules/DOS/dos.h"
//#include "../../Rules/DOS/dosoutput_simpeltext.h"
//#include "../../Rules/DOS/dosoutput_dfxml.h"

#include <fstream>
#include <memory>
#include <vector>

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
    imagePath.append(slash + "TestImages" + slash + "DOS" + slash);

    //Read in image name
    std::string imageName;
    imageName = "ext-part-test-2.dd";
    imagePath.append(imageName);

    //Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);
        AddressTreeRoot tree(rawData);
        storageObjects input;
        std::vector<std::pair<uint64_t, uint64_t>> helper;
        helper.push_back(std::make_pair(0, rawData->size()));
        input.contentdata.push_back(helper);
        AddressNodeList output;

        Rule* r1;
        r1 = new DOS();
        AddressNodeList in_nodes;
        in_nodes.push_back(tree.tree);
        output = r1->evaluate(in_nodes);
        tree.print();
        /*
        for (auto it = output.contentdata.begin(); it != output.contentdata.end(); ++it) {
            for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
                std::cout << "start: " << it2->first << " end: " << it2->second;
                std::cout << " start: " << it2->first/512 << " end: " << it2->second/512;
            }
            std::cout << std::endl;
        }

        Outputrule* oh1;
        oh1 = new DOSOutput_SimpelText(rawData);
//        oh1 = new DOSOutput_dfxml(rawData);
        oh1->evaluate(&output);
        */
        imageStream.close();
    }

    return 0;
}
