#include "carve.h"

#include <iostream>
#include <algorithm>

Carve::Carve()
{
    initSignatures();
}

Carve::~Carve() {}

AddressNodeList Carve::evaluate(AddressNodeList input)
{
    AddressNodeList result;

    for (AddressNodePtr& node : input) {
        AddressNodeList new_children;

        std::pair<uint64_t, uint64_t> contentData = node->m_data.front().front();
        uint64_t lowerBound = contentData.first;
        uint64_t upperBound = contentData.second;

        std::cout << "[carve] evaluating node, range [" << lowerBound
            << " - " << upperBound << "]" << std::endl;

        try {
            size_t maxHeaderLen = 0;
            for (const Signature& sig : m_signatures)
                maxHeaderLen = std::max(maxHeaderLen, sig.header.size());
            uint64_t overlap = maxHeaderLen > 0 ? (uint64_t)maxHeaderLen - 1 : 0;

            uint64_t chunkStart = lowerBound;

            while (chunkStart <= upperBound) {
                uint64_t chunkEnd = std::min(upperBound, chunkStart + CHUNK_SIZE - 1);

                std::cout << "[carve] scanning chunk [" << chunkStart
                    << " - " << chunkEnd << "]" << std::endl;

                std::vector<unsigned char> buffer = node->root->data->copy2Vector(chunkStart, chunkEnd);

                for (size_t i = 0; i < buffer.size(); i++) {
                    uint64_t absPos = chunkStart + i;

                    for (const Signature& sig : m_signatures) {
                        if (sig.header.empty() || i + sig.header.size() > buffer.size())
                            continue;

                        if (!matchPattern(&buffer[i], sig.header))
                            continue;

                        std::cout << "[carve] header '" << sig.name
                            << "' found at offset " << absPos << std::endl;

                        uint64_t start = absPos;
                        uint64_t end = 0;
                        bool found = false;

                        if (!sig.footer.empty()) {
                            uint64_t searchLimit = std::min(upperBound, start + sig.maxCarveSize);
                            uint64_t searchStart = start + sig.header.size();

                            std::cout << "[carve] searching for footer between "
                                << searchStart << " and " << searchLimit << std::endl;

                            uint64_t footerPos = 0;
                            if (searchStart <= searchLimit &&
                                findPattern(node, searchStart, searchLimit, sig.footer, footerPos)) {
                                end = footerPos + sig.footer.size() - 1;
                                found = true;
                                std::cout << "[carve] footer found at offset "
                                    << footerPos << std::endl;
                            }
                            else {
                                std::cout << "[carve] no footer found for '" << sig.name
                                    << "' starting at " << start
                                    << ", skipping" << std::endl;
                            }
                        }
                        else {
                            end = std::min(upperBound, start + sig.maxCarveSize - 1);
                            found = true;
                            std::cout << "[carve] no footer defined for '" << sig.name
                                << "', using max carve size, end offset " << end << std::endl;
                        }

                        if (found) {
                            std::cout << "[carve] carved file '" << sig.name
                                << "' [" << start << " - " << end << "] ("
                                << (end - start + 1) << " bytes)" << std::endl;

                            std::string tag = "carve_" + sig.name;

                            std::vector<std::pair<uint64_t, uint64_t>> contentSeqence;
                            std::vector<std::vector<std::pair<uint64_t, uint64_t>>> contentSeqences;
                            std::vector<std::pair<uint64_t, uint64_t>> metadataSeqence;
                            std::vector<std::vector<std::pair<uint64_t, uint64_t>>> metadataSeqences;

                            contentSeqence.push_back(std::make_pair(start, end));
                            contentSeqences.push_back(contentSeqence);
                            metadataSeqences.push_back(metadataSeqence);

                            AddressNodePtr new_node = std::make_shared<reconstructionNode>(contentSeqences, metadataSeqences, tag);
                            new_children.push_back(new_node);

                        }
                    }
                }

                if (chunkEnd == upperBound)
                    break;

                chunkStart = chunkEnd - overlap + 1;
            }

            node->add_children(new_children);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }

        result.insert(result.end(), new_children.begin(), new_children.end());
        new_children.clear();
    }

    return result;
}

bool Carve::findPattern(AddressNodePtr& node, uint64_t searchStart, uint64_t searchEnd, const std::vector<unsigned char>& pattern, uint64_t& foundAt)
{
    if (pattern.empty() || searchStart > searchEnd)
        return false;

    uint64_t overlap = pattern.size() - 1;
    uint64_t chunkStart = searchStart;

    std::cout << "[carve]   findPattern scanning [" << searchStart
        << " - " << searchEnd << "] in " << CHUNK_SIZE << "-byte chunks" << std::endl;

    while (chunkStart <= searchEnd) {
        uint64_t chunkEnd = std::min(searchEnd, chunkStart + CHUNK_SIZE - 1);
        std::vector<unsigned char> buffer = node->root->data->copy2Vector(chunkStart, chunkEnd);

        if (buffer.size() >= pattern.size()) {
            size_t maxIdx = buffer.size() - pattern.size();
            for (size_t i = 0; i <= maxIdx; ++i) {
                if (matchPattern(&buffer[i], pattern)) {
                    foundAt = chunkStart + i;
                    return true;
                }
            }
        }

        if (chunkEnd == searchEnd)
            break;

        chunkStart = chunkEnd - overlap + 1;
    }

    return false;
}

bool Carve::matchPattern(const unsigned char* buffer, const std::vector<unsigned char>& pattern)
{
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (pattern[i] != '?' && buffer[i] != pattern[i]) {
            return false;
        }
    }
    return true;
}

void Carve::initSignatures()
{
    std::cout << "[carve] initializing signatures..." << std::endl;

    m_signatures.clear();

    // ---------------------------------------------------------------------
    // GRAPHICS FILES
    // ---------------------------------------------------------------------
    // gif
    m_signatures.push_back({
        "gif",
        { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 }, // GIF87a
        { 0x00, 0x3B },
        5000000
        });
    m_signatures.push_back({
        "gif",
        { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 }, // GIF89a
        { 0x00, 0x00, 0x3B },
        5000000
        });

    // jpg
    m_signatures.push_back({
        "jpg",
        { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10 },
        { 0xFF, 0xD9 },
        200000000
        });

    // png ('?' fungiert als Wildcard)
    m_signatures.push_back({
        "png",
        { 0x50, 0x4E, 0x47, '?' },
        { 0xFF, 0xFC, 0xFD, 0xFE },
        20000000
        });

    // tif (Keine Footer in Config definiert)
    m_signatures.push_back({
        "tif",
        { 0x49, 0x49, 0x2A, 0x00 },
        {},
        200000000
        });
    m_signatures.push_back({
        "tif",
        { 0x4D, 0x4D, 0x00, 0x2A },
        {},
        200000000
        });

    // ---------------------------------------------------------------------
    // VIDEO AND AUDIO FILES
    // ---------------------------------------------------------------------
    // avi ('?' als Wildcard)
    m_signatures.push_back({
        "avi",
        { 'R', 'I', 'F', 'F', '?', '?', '?', '?', 'A', 'V', 'I' },
        {},
        50000000
        });

    // mpg
    m_signatures.push_back({
        "mpg",
        { 0x00, 0x00, 0x01, 0xBA },
        { 0x00, 0x00, 0x01, 0xB9 },
        50000000
        });
    m_signatures.push_back({
        "mpg",
        { 0x00, 0x00, 0x01, 0xB3 },
        { 0x00, 0x00, 0x01, 0xB7 },
        50000000
        });

    // fws (Flash)
    m_signatures.push_back({
        "fws",
        { 'F', 'W', 'S' },
        {},
        4000000
        });

    // wav ('?' als Wildcard)
    m_signatures.push_back({
        "wav",
        { 'R', 'I', 'F', 'F', '?', '?', '?', '?', 'W', 'A', 'V', 'E' },
        {},
        200000
        });

    // ---------------------------------------------------------------------
    // MICROSOFT OFFICE & MAIL
    // ---------------------------------------------------------------------
    // doc (NEXT Keyword-Variante: Header dient zeitgleich als Footer-Stopper)
    m_signatures.push_back({
        "doc",
        { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1, 0x00, 0x00 },
        { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1, 0x00, 0x00 },
        10000000
        });
    m_signatures.push_back({
        "doc",
        { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1 },
        {},
        10000000
        });

    // pst / ost
    m_signatures.push_back({
        "pst",
        { 0x21, 0x42, 0x4E, 0xA5, 0x6F, 0xB5, 0xA6 },
        {},
        500000000
        });
    m_signatures.push_back({
        "ost",
        { 0x21, 0x42, 0x44, 0x4E },
        {},
        500000000
        });

    // Outlook Express
    m_signatures.push_back({
        "dbx",
        { 0xCF, 0xAD, 0x12, 0xFE, 0xC5, 0xFD, 0x74, 0x6F },
        {},
        10000000
        });
    m_signatures.push_back({
        "idx",
        { 0x4A, 0x4D, 0x46, 0x39 },
        {},
        10000000
        });
    m_signatures.push_back({
        "mbx",
        { 0x4A, 0x4D, 0x46, 0x36 },
        {},
        10000000
        });

    // ---------------------------------------------------------------------
    // HTML, PDF, ARCHIVES & MISC
    // ---------------------------------------------------------------------
    // htm (Case-Insensitive in Config, hier Kleinbuchstaben)
    m_signatures.push_back({
        "htm",
        { '<', 'h', 't', 'm', 'l' },
        { '<', '/', 'h', 't', 'm', 'l', '>' },
        50000
        });

    // pdf
    m_signatures.push_back({
        "pdf",
        { '%', 'P', 'D', 'F' },
        { '%', '%', 'E', 'O', 'F', 0x0D },
        5000000
        });
    m_signatures.push_back({
        "pdf",
        { '%', 'P', 'D', 'F' },
        { '%', '%', 'E', 'O', 'F', 0x0A },
        5000000
        });

    // zip
    m_signatures.push_back({
        "zip",
        { 'P', 'K', 0x03, 0x04 },
        { 0x3C, 0xAC },
        10000000
        });

    // java
    m_signatures.push_back({
        "java",
        { 0xCA, 0xFE, 0xBA, 0xBE },
        {},
        1000000
        });

    // tgz
    m_signatures.push_back({
        "tgz",
        { 0x1F, 0x8B, 0x08, 0x08 },
        {},
        2000000
        });

    // ogg
    m_signatures.push_back({
        "ogg",
        { 0x4F, 0x67, 0x67, 0x53, 0x00, 0x02 },
        { 0x4F, 0x67, 0x67, 0x53, 0x00, 0x02 },
        15728640
        });

    std::cout << "[carve] loaded " << m_signatures.size() << " signature(s)" << std::endl;
}