#ifndef DOS_H
#define DOS_H

#include "../../rule.h"
#include <vector>

/**
 * \brief DOS evaluation rule
 */
class DOS : public Rule
{
public:
    DOS();
    ~DOS();

    /**
     * \brief Evaluates the input AddressNodes to identify block sequences representing DOS partitions
     * \param input Input AddressNodes whose m_data ranges are searched for DOS partition structures
     * \return AddressNodeList containing reconstructionNodes representing DOS partitions and their corresponding metadata
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    /**
     * \brief Extended partition handling function
     * \param offset Partition offset in byte
     * \param input The AddressNode representing the extended partition, whose byte source is read to locate nested partitions
     * \return AddressNodeList containing reconstructionNodes representing the DOS partitions found within the extended partition, and their corresponding metadata
     */
    AddressNodeList handleExtended(uint64_t offset, AddressNodePtr input);

};

#endif // DOS_H
