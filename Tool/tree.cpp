#include "tree.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstdint>

#include <fstream>
/**
 * @brief Helper function to recursively serialize the node graph into Graphviz DOT format including data and metadata.
 *
 * @param ss Stream storing the formatted DOT file content
 * @param visited Set of visited nodes to handle directed graphs and prevent infinite loops
 * @return void
 */
void AddressNode::export_dot_helper(std::ostringstream& ss, std::unordered_set<const AddressNode*>& visited) const
{
  if (visited.count(this)) return;
  visited.insert(this);
  uintptr_t nodeId = reinterpret_cast<uintptr_t>(this);
  ss << "  node_" << nodeId << " [label=\"" << m_tag;
  // Append Data range safely
  if (!m_data.empty() && !m_data.front().empty() && !m_data.back().empty()) {
    auto start_addr = m_data.front().front().first;
    auto end_addr = m_data.back().back().second;
    ss << "\\nData: (" << start_addr << "-" << end_addr << ")";
  }
  // Append Metadata range safely
  if (!m_metadata.empty() && !m_metadata.front().empty() && !m_metadata.back().empty()) {
    auto meta_start = m_metadata.front().front().first;
    auto meta_end = m_metadata.back().back().second;
    ss << "\\nMeta: (" << meta_start << "-" << meta_end << ")";
  }
  ss << "\"];\n";
  if (m_children) {
    for (const auto& child : *m_children) {
      if (child) { // Always guard child pointers
        uintptr_t childId = reinterpret_cast<uintptr_t>(child.get());
        ss << "  node_" << nodeId << " -> node_" << childId << ";\n";
        child->export_dot_helper(ss, visited);
      }
    }
  }
}
/**
 * @brief Export the address tree rooted at this node to a Graphviz DOT file for visualization.
 *
 * @param filename Path to the output file where the DOT graph will be saved
 * @return void
 */
void AddressNode::save_as_dot(const std::string& filename) const
{
  std::ostringstream ss;
  std::unordered_set<const AddressNode*> visited;
  ss << "digraph AddressTree {\n";
  ss << "  node [shape=box, style=filled, fillcolor=lightgoldenrodyellow, fontname=\"Helvetica\"];\n";
  ss << "  rankdir=TB;\n";
  export_dot_helper(ss, visited);
  ss << "}\n";
  std::ofstream file(filename);
  file << ss.str();
  file.close();
}


/**
 * @brief Construct a new Address Node:: Address Node object
 *
 * @param data a vector of data objects
 * @param metadata a vector of metadata objects
 * @param tag this tag identifies which rule created the node
 */

AddressNode::AddressNode(
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>> data,
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadata,
  std::string tag)
  : m_data(data), m_metadata(metadata), m_tag(std::move(tag))
{
};

/**
 * @param children a vector of children to be added to this node
 */
void AddressNode::add_children(AddressNodeList children)
{
  if (!m_children) {
    m_children = std::make_shared<std::vector<std::shared_ptr<AddressNode>>>();
  }

  for (std::shared_ptr<AddressNode>& child : children) {
    child->add_parent(this);
    child->root = this->root;
    child->depth = this->depth + 1;
    m_children->push_back(std::move(child));
  }

  if (this->root && this->root->depth < this->depth + 1) {
    this->root->depth = this->depth + 1;
  }
}
/**
 * @brief recursively construct and print the tree
 *
 * @param prefix the already constructed string. First call is with an empty string
 */

void AddressNode::print(const std::string& prefix) const
{
  if (this->m_data.empty()) return;

  // Static set speichert besuchte Knoten während der Rekursion
  static std::unordered_set<const AddressNode*> visited;

  auto start_addr = m_data.begin()->begin()->first;
  auto end_addr = m_data.back().back().second;

  if (visited.count(this)) {
    std::cout << prefix << m_tag << "(" << start_addr << "-" << end_addr
      << ") [DAG: bereits gedruckt]" << std::endl;
    return;
  }

  visited.insert(this);

  std::cout << prefix << m_tag << "(" << start_addr << "-" << end_addr
    << "), Depth=" << depth << std::endl;

  if (m_children && !m_children->empty()) {
    size_t total_children = m_children->size();
    for (size_t i = 0; i < total_children; ++i) {
      bool is_last = (i == total_children - 1);
      std::string child_prefix = prefix + (is_last ? "    └── " : "    ├── ");

      (*m_children)[i]->print(child_prefix);
    }
  }

  if (prefix.empty()) {
    visited.clear();
  }
}

/**
 * @brief update the parent pointer of this node
 *
 * @param parent the parent pointer to use
 */
void AddressNode::add_parent(AddressNode* parent)
{
  m_parents.push_back(parent);
}

/**
 * @brief returns a list of AddressNodes by comparing their depth
 *
 * @param level the depth of the AddressNodes to be returned
 * @return std::vector<AddressNode *> a list of non-owning pointers to AddressNodes
 */
std::vector<AddressNode*> AddressNode::getLayer(uint64_t level) const
{
  std::vector<AddressNode*> collector;

  if (!m_children) {
    return collector;
  }

  if (this->depth == level - 1) {
    for (const std::shared_ptr<AddressNode>& child : *m_children) {
      collector.push_back(child.get());
    }
    return collector;
  }

  for (const std::shared_ptr<AddressNode>& child : *m_children) {
    std::vector<AddressNode*> temp_result = child->getLayer(level);
    collector.insert(collector.end(), temp_result.begin(), temp_result.end());
  }

  return collector;
}

/**
 * @brief recursively search the tree and return AddressNodes that have a certain tag
 *
 * @param tag the tag to look for
 * @return std::vector<AddressNode *> a list of non-owning pointers to AddressNodes containing the searched tag
 */
std::vector<AddressNode*> AddressNode::getNodes(const std::string& tag)
{
  std::vector<AddressNode*> result;
  if (m_tag == tag) {
    result.push_back(this);
  }
  for (std::shared_ptr<AddressNode>& child : *m_children) {
    std::vector<AddressNode*> temp = child->getNodes(tag);
    result.insert(result.end(), temp.begin(), temp.end());
  }
  return result;
}

/**
 * @brief recursively search the tree for AddressNodes whose data addresses are in a given range
 *
 * @param range a pair containing of the start and end address in which to search for nodes
 * @return std::vector<AddressNode *> a list of non-owning pointers to AddressNodes in the given range
 */
std::vector<AddressNode*>
AddressNode::getNodes(std::pair<uint64_t, uint64_t> range)
{
  std::vector<AddressNode*> result;
  for (std::vector<std::pair<uint64_t, uint64_t>> block : m_data) {
    for (std::pair<uint64_t, uint64_t> fragment : block) {
      if (fragment.first >= range.first && fragment.second <= range.second) {
        result.push_back(this);
      }
      for (std::shared_ptr<AddressNode>& child : *m_children) {
        if (strcmp(typeid(child).name(), "reconstructionNode") != 0) {
          std::vector<AddressNode*> temp = child->getNodes(range);
          result.insert(result.end(), temp.begin(), temp.end());
        }
        else {
          if (fragment.first >= range.first &&
            fragment.second <= range.second) {
            for (AddressNodePtr& new_child : *child->m_children) {
              result.push_back(new_child.get());
            }
          }
        }
      }
    }
  }
  return result;
}

/**
 * \brief Generates a formatted string representation of all interval ranges.
 * \param data The nested vector containing start and end offset pairs.
 * \return std::string Formatted range string
 */
std::string AddressNode::formatRanges(const std::vector<std::vector<std::pair<uint64_t, uint64_t>>>& data) const
{
  if (data.empty()) {
    return "()";
  }

  std::string result = "";
  bool first = true;

  for (const auto& outer_vec : data) {
    for (const auto& range : outer_vec) {
      if (!first) {
        result += ", ";
      }
      result += "(" + std::to_string(range.first) + ", " + std::to_string(range.second) + ")";
      first = false;
    }
  }

  return result.empty() ? "()" : result;
}

/**
 * \brief Recursive helper function to collect all paths from the current node up to the root nodes.
 * \param node The current node being processed in the recursion step.
 * \param current_path Reference to the vector storing the path currently being built.
 * \param all_traces Reference to the collection of all resolved root-to-node paths.
 * \param visited Set used to prevent infinite recursion in case of cyclic dependencies.
 */
void AddressNode::getTracePaths(const AddressNode* node,
  std::vector<std::string>& current_path,
  std::vector<std::vector<std::string>>& all_traces,
  std::unordered_set<const AddressNode*>& visited) const
{
  if (!node) return;

  std::string node_info = node->m_tag + formatRanges(node->m_data);
  current_path.push_back(node_info);
  visited.insert(node);

  if (node->m_parents.empty()) {
    std::vector<std::string> forward_path = current_path;
    std::reverse(forward_path.begin(), forward_path.end());
    all_traces.push_back(forward_path);
  }
  else {
    for (const AddressNode* parent : node->m_parents) {
      if (visited.find(parent) == visited.end()) {
        getTracePaths(parent, current_path, all_traces, visited);
      }
    }
  }

  current_path.pop_back();
  visited.erase(node);
}

/**
 * \brief Generates a complete text representation of the evaluation trace for this node.
 * \return std::string The formatted trace string across one or multiple branches.
 */
std::string AddressNode::getTrace() const
{
  std::ostringstream ss;
  std::vector<std::string> current_path;
  std::vector<std::vector<std::string>> all_traces;
  std::unordered_set<const AddressNode*> visited;

  getTracePaths(this, current_path, all_traces, visited);

  if (all_traces.empty()) {
    return "";
  }

  if (all_traces.size() == 1) {
    for (size_t i = 0; i < all_traces[0].size(); ++i) {
      ss << all_traces[0][i];
      if (i + 1 < all_traces[0].size()) ss << " — ";
    }
  }
  else {
    for (size_t b = 0; b < all_traces.size(); ++b) {
      if (b > 0) ss << "\n";
      ss << "Branch " << b + 1 << ": ";
      for (size_t i = 0; i < all_traces[b].size(); ++i) {
        ss << all_traces[b][i];
        if (i + 1 < all_traces[b].size()) ss << " —> ";
      }
    }
  }

  return ss.str();
}

/**
 * @brief Traverses up the node hierarchy to retrieve the underlying raw byte data source.
 *
 * Recursively ascends through the parent nodes until reaching the root node,
 * returning the pointer to the root's ByteContainer.
 *
 * @return ByteContainer* Pointer to the raw root ByteContainer, or nullptr if no parent/root exists.
 */

 // Info: If a node has multiple parents (e.g., union/intersection), and those parents are of a different node type, 
 //       this function will always pick the source of the first parent.
ByteContainer* AddressNode::getByteSource() const
{
  if (m_parents.empty()) {
    return root ? root->data : nullptr;
  }
  return m_parents[0]->getByteSource();
}

/**
 * @brief Construct a new reconstruction Node::reconstruction Node object
 *
 * @param data a vector of data objects
 * @param metadata a vector of metadata objects
 * @param tag this tag identifies which rule created the node
 */
reconstructionNode::reconstructionNode(
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>> data,
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadata,
  std::string tag)
  : AddressNode(data, metadata, std::move(tag))
{
};

/**
 * @brief Construct a new transformation Node::transformation Node object
 *
 * @param data a vector of data objects
 * @param metadata a vector of metadata objects
 * @param tag this tag identifies which rule created the node
 * @param container Shared pointer to the ByteContainer holding transformed data bytes
 */
transformationNode::transformationNode(
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>> data,
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadata,
  std::string tag,
  std::shared_ptr<ByteContainer> container)
  : AddressNode(data, metadata, std::move(tag)), transformationContainer(std::move(container))
{
};

/**
 * @brief Overrides the default hierarchy traversal to provide the node's own transformed byte container.
 *
 * Unlike standard AddressNodes that recurse up to the root data source, a transformationNode
 * acts as a new localized data source containing transformed (e.g., decompressed, decrypted) byte ranges.
 *
 * @return ByteContainer* Raw pointer to the internal transformed ByteContainer managed by this node.
 */
ByteContainer* transformationNode::getByteSource() const
{
  return transformationContainer.get();
}

/**
 * @brief Construct a new Address Tree Root:: Address Tree Root object
 *
 * @param indata a ByteContainer containing the raw input data
 */
AddressTreeRoot::AddressTreeRoot(ByteContainer* indata) : data(indata)
{
  std::vector<std::pair<uint64_t, uint64_t>> temp_first_node_data_address;
  temp_first_node_data_address.push_back(std::pair<uint64_t, uint64_t>(0, indata->size()));
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>>first_node_data_address;
  first_node_data_address.push_back(temp_first_node_data_address);
  std::vector<std::vector<std::pair<uint64_t, uint64_t>>>first_node_metadata_address;
  tree = std::make_shared<AddressNode>(first_node_data_address, first_node_metadata_address, "root");
  tree->root = this;
}

// utility function to print the whole tree using the member function of the
// nodes
/**
 * @brief calls the print function of the root AddressNode
 *
 */
void AddressTreeRoot::print() const { tree->print(""); }

/**
 * @brief calls the getLayer function in the root AddressNode
 *
 * @param level the depth of the nodes to be returned
 * @return std::vector<AddressNode *> a vector of non-owning pointers to the AddressNodes in the given depth
 */
std::vector<AddressNode*> AddressTreeRoot::getLayer(uint64_t level) const
{
  return tree->getLayer(level);
}

/**
 * @brief calls the getNodes function in the root AddressNode
 *
 * @param tag the tag to look for
 * @return std::vector<AddressNode *> a vector of non-owning pointers to AddressNodes containing the given string
 */
std::vector<AddressNode*>
AddressTreeRoot::getNodes(const std::string& tag) const
{
  return tree->getNodes(tag);
}
/**
 * @brief calls the getNodes function in the root AddressNode
 *
 * @param range a pair containing of the start and end address in which to search for nodes
 * @return std::vector<AddressNode *> a list of non-owning pointers to AddressNodes in the given range
 */
std::vector<AddressNode*>
AddressTreeRoot::getNodes(std::pair<uint64_t, uint64_t> range) const
{
  return tree->getNodes(range);
}
