#include "../getworkingdir.h"
#include "../tree.h"
#include "../Rules/DOS/dos.h"
#include "../Rules/Ext3/ext3_fsl.h"
#include "../Rules/Ext3/ext3_fl.h"
#include "../Rules/Ext4/ext4_fsl.h"
#include "../Rules/Ext4/ext4_fl.h"
#include "../Rules/Carve/carve.h"
#include "../SequentialOperators/seqcomp.h"
#include "../SequentialOperators/or.h"
#include "../ParallelOperators/union.h"

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
    imagePath.append(slash + "TestImages" + slash + "DOS" + slash);

    // Read in image name
    std::string imageName;
    imageName = "sd-image-2.dd";
    imagePath.append(imageName);

    // Open binary ifstream
    std::ifstream imageStream(imagePath, std::ios::binary);


    if (imageStream.is_open()) {
        std::cout << imageName + ":   " + imagePath << std::endl;
    }
    else {
        std::cerr << "Could not open file: " << imagePath << std::endl;
        return 1;
    }

    if (imageStream.is_open()) {
        ByteContainer* rawData;
        rawData = new RootContainer(imageStream);

        AddressTreeRoot tree(rawData);
        AddressNodeList input;
        input.push_back(tree.tree);
        AddressNodeList output;

        Rule* dos = new DOS();
        AddressNodeList dos_result = dos->evaluate(input);

        // User chooses the first dos partition
        AddressNodeList selected_partition;
        selected_partition.push_back(dos_result[1]);
        std::cout << "used dos to evaluate partitions" << std::endl;

        Rule* ext3_fsl = new Ext3_fsl();
        Rule* ext3_fl = new Ext3_fl();
        Rule* ext3_chain = new SeqCompSingle(ext3_fsl, ext3_fl);

        Rule* ext4_fsl = new Ext4_fsl();
        Rule* ext4_fl = new Ext4_fl();
        Rule* ext4_chain = new SeqCompSingle(ext4_fsl, ext4_fl);

        Rule* carve = new Carve();

        Union union_op(new Or(ext3_chain, ext4_chain), carve);
        AddressNodeList result = union_op.evaluate(selected_partition);
        std::cout << "used union operator on carve results and (ext3 || ext4) results" << std::endl;

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

        std::cout << "\nFinal result (Ext3 || Ext4) + Carve: " << result.size() << " node(s)" << std::endl;
        for (auto& node : result) {
            std::cout << "Node (tag: " << node->m_tag << "):" << std::endl;
            for (auto& sequence : node->m_data) {
                for (auto& pair : sequence) {
                    std::cout << "  start: " << pair.first << " end: " << pair.second << std::endl;
                }
            }
        }


        std::cout << "\n===== Tree Structure =====" << std::endl;
        tree.print();

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
        return 0;
    }
}