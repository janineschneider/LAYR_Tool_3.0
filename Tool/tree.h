#ifndef TREE_H
#define TREE_H

#include "rootcontainer.h"
#include "bytecontainer.h"
#include "transformationcontainer.h"
#include <cstdint>
#include <memory>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <unordered_set>

class AddressNode;

// Type aliases for clarity
using AddressNodePtr = std::shared_ptr<AddressNode>;
using AddressNodeList = std::vector<AddressNodePtr>;
class AddressTreeRoot;


/**
 * @brief This class represents a node in the address tree, referencing byte ranges
 *        (addresses) within a ByteContainer along with associated metadata.
 *        Serves as the base class for reconstructionNode and transformationNode.
 * @author Timo Heimann
 * @author Marie Becker
 * @date August, 2026
 */
class AddressNode
{
public:
    uint64_t depth = 0;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> m_data;
    std::vector<std::vector<std::pair<uint64_t, uint64_t>>> m_metadata;
    std::shared_ptr<AddressNodeList> m_children = std::make_shared<AddressNodeList>();
    std::vector<AddressNode*> m_parents;
    std::string m_tag;
    AddressTreeRoot* root;

    AddressNode(std::vector<std::vector<std::pair<uint64_t, uint64_t>>>data, std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadata, std::string tag);


    void print(const std::string& prefix = "") const;
    void add_children(AddressNodeList children);
    void add_parent(AddressNode* parent);

    std::vector<AddressNode*> getLayer(uint64_t level) const;
    std::vector<AddressNode*> getNodes(const std::string& tag);
    std::vector<AddressNode*> getNodes(std::pair<uint64_t, uint64_t> range);
    std::string getTrace() const;

    virtual ByteContainer* getByteSource() const;

    virtual ~AddressNode() = default;

    // Delete copy constructor and copy assignment
    AddressNode(const AddressNode&) = delete;
    AddressNode& operator=(const AddressNode&) = delete;

    // Allow move
    AddressNode(AddressNode&&) noexcept = default;
    AddressNode& operator=(AddressNode&&) noexcept = default;

    // Graphviz DOT export functions
    void export_dot_helper(std::ostringstream& ss, std::unordered_set<const AddressNode*>& visited) const;
    void save_as_dot(const std::string& filename) const;

private:
    std::string formatRanges(const std::vector<std::vector<std::pair<uint64_t, uint64_t>>>& data) const;
    void getTracePaths(const AddressNode* node, std::vector<std::string>& current_path, std::vector<std::vector<std::string>>& all_traces, std::unordered_set<const AddressNode*>& visited) const;
};

/**
 * @brief reconstructionNode represents a node created by a reconstruction rule.
 * @author Timo Heimann
 * @date August 2025
 */
class reconstructionNode : public AddressNode
{
public:
    reconstructionNode(std::vector<std::vector<std::pair<uint64_t,
        uint64_t>>>data, std::vector<std::vector<std::pair<uint64_t,
        uint64_t>>> metadata,
        std::string tag);
};

/**
 * @brief transformationNode represents a node created by a transformation rule.
 * @author Timo Heimann
 * @author Marie Becker
 * @date August 2026
 */
class transformationNode : public AddressNode
{
public:
    transformationNode(std::vector<std::vector<std::pair<uint64_t, uint64_t>>>data,
        std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadata,
        std::string tag,
        std::shared_ptr<ByteContainer> container);

    std::shared_ptr<ByteContainer> transformationContainer;

    ByteContainer* getByteSource() const override;
};

/**
 * @brief Handles the tree of AddressNodes, holding the root ByteContainer for the input
 *        and metadata about the tree
 * @author Timo Heimann
 * @date August 2025
 */
class AddressTreeRoot
{
public:
    std::shared_ptr<AddressNode> tree;
    ByteContainer* data;
    uint64_t depth = 0;
    AddressTreeRoot(ByteContainer* data);

    void print() const;
    std::vector<AddressNode*> getLayer(uint64_t level) const;
    std::vector<AddressNode*> getNodes(const std::string& tag) const;
    std::vector<AddressNode*> getNodes(std::pair<uint64_t, uint64_t> range) const;
};

#endif // TREE_H
