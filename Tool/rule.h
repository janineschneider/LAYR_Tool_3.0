#ifndef RULE_H
#define RULE_H

#include "tree.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <cstdint>
#include "tree.h"
#include <map>

/**
 * \brief Rule base class
 * \author Janine Schneider
 * @author Timo Heimann
 * \date August, 2025
 */
class Rule
{
public:
    Rule() {};
    virtual ~Rule() noexcept {}

    /**
     * \brief Evaluates the given AddressNodes to identify block sequences representing data
     * \param input AddressNodeList containing the nodes to evaluate, typically the tree's root node or the output of a preceding rule
     * \return AddressNodeList containing the created nodes (reconstructionNode or transformationNode instances)
     */
    virtual AddressNodeList evaluate(AddressNodeList input) = 0;
};

#endif // RULE_H
