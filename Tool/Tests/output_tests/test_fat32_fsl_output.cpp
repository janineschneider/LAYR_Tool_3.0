#include "../../getworkingdir.h"
#include "../../tree.h"
#include "../../Rules/FAT32/fat32_fsl.h"
#include "../../Rules/FAT32/fat32_fsl_dfxml.h"

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
    std::cout << "searching for testimage in path: " << imagePath << std::endl;
    //Open binary ifstream
    ifstream imageStream(imagePath, ios::binary);

    if (imageStream.is_open()) {
        std::cout << "image stream opened" << std::endl;
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        std::vector<std::pair<uint64_t, uint64_t>> helper;
        helper.push_back(std::make_pair(0, rawData->size()));
        AddressNodeList output;

        Rule* h1;
        h1 = new FAT32_fsl();
        AddressNodeList input;
        input.push_back(tree.tree);
        output = h1->evaluate(input);
        tree.print();

        imageStream.close();
    }

    imageStream.close();

    return 0;
}

