#ifndef FAT32_FL_DFXML_H
#define FAT32_FL_DFXML_H

#include "../../rule.h"
#include "../DFXML/dfxml_writer.h"


class Fat32_fl_dfxml : public Rule
{
public:
    Fat32_fl_dfxml();
    ~Fat32_fl_dfxml();

    AddressNodeList evaluate(AddressNodeList input);

private:
    ByteContainer* m_rawData;

    //Dynamic Metadata Boundaries
    uint64_t lowerBound;
    uint64_t upperBound;
    uint64_t indexOffset;

    //DFXML Handler
    dfxml_writer outputWriter;

    //Filter out the information from the metadata and store into XML output
    void storeFileName();
    void storeCreationTime();
    void storeCreationDate();
    void storeLastAccessedDate();
    void storeLastWrittenTime();
    void storeLastWrittenDate();
    void storeFileSize();
    void storeFirstCluster();

    //Helpers
    std::string hex2Time(unsigned char hex[], bool tenth);
    std::string hex2Date(unsigned char hex[]);
};

#endif // FAT32_FL_DFXML_H