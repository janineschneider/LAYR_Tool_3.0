#ifndef PARALLEL_H
#define PARALLEL_H

#include "rule.h"

/**
 * \brief Parallel operator base class
 * \inherits rule
 * \author Janine Schneider
 * \date January, 2019
 * \author Marie Becker
 * \date July, 2026
 */
class Parallel : public Rule
{
public:
  Parallel();
  Parallel(Rule* r1, Rule* r2) : m_r1(r1), m_r2(r2) {};
  ~Parallel()
  {
    if (m_r1 != nullptr) {
      delete m_r1;
    }

    if (m_r2 != nullptr) {
      delete m_r2;
    }
  }

  /**
     * \brief Set operator function (union, intersection, minus)
     * \param res1 The results of r1
     * \param res2 The results of r2
     * \return AddressNodeList representing the combined data and corresponding metadata
     */
  virtual AddressNodeList setOperator(AddressNodeList res1, AddressNodeList res2) = 0;

  /**
     * \brief Rule combination operator; evaluates both rules on the input and combines the results
     * \param input Input AddressNodes to be evaluated by both r1 and r2
     * \return AddressNodeList representing the combined data and corresponding metadata
     */
  AddressNodeList evaluate(AddressNodeList input)
  {
    AddressNodeList res1 = m_r1->evaluate(input);
    AddressNodeList res2 = m_r2->evaluate(input);
    return setOperator(res1, res2);
  };

private:
  Rule* m_r1;
  Rule* m_r2;
};

#endif // PARALLEL_H
