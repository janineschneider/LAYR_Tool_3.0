#ifndef carve_H
#define carve_H

#include "../../rule.h"
#include <vector>
#include <string>
#include <cstdint>

/**
 * \brief Rule that performs file carving on input nodes using file signature headers and footers.
 */
class Carve : public Rule
{
public:
    Carve();
    ~Carve();

    /**
     * \brief Scans input address nodes' data ranges for file headers and footers,
     *        generating child nodes for each carved file.
     * \param input List of parent address nodes to evaluate and scan.
     * \return AddressNodeList List of generated child nodes representing reconstructed files.
     */
    AddressNodeList evaluate(AddressNodeList input);

private:
    /**
     * \brief Describes a single carvable file type.
     * \param name used as part of the reconstruction tag
     * \param header byte sequence marking file start
     * \param footer byte sequence marking file end; empty = no footer
     * \param maxCarveSize hard cap in bytes (also used as fallback size if
     */
    struct Signature
    {
        std::string name;
        std::vector<unsigned char> header;
        std::vector<unsigned char> footer;
        uint64_t maxCarveSize;
    };

    std::vector<Signature> m_signatures;

    // Size of each bulk read while scanning for headers.
    static const uint64_t CHUNK_SIZE = 16 * 1024 * 1024; // 16 MB

    /**
     * \brief Fills m_signatures with a small built-in set of common file types.
     */
    void initSignatures();

    /**
     * \brief Searches the underlying ByteContainer across the given address range
     *        for a byte pattern using chunked buffers.
     * \param node Target address node containing root container context.
     * \param searchStart Starting byte offset (inclusive).
     * \param searchEnd Ending byte offset (inclusive).
     * \param pattern Byte sequence to match.
     * \param foundAt Absolute byte offset where the pattern match begins.
     * \return true if the pattern was found within the range, false otherwise.
     */
    bool findPattern(AddressNodePtr& node, uint64_t searchStart, uint64_t searchEnd, const std::vector<unsigned char>& pattern, uint64_t& foundAt);

    /**
     * \brief Compares a data buffer against a byte pattern, treating '?' as a wildcard byte.
     * \param buffer Pointer to raw bytes (must be at least pattern.size() in length).
     * \param pattern Byte sequence to match against buffer.
     * \return true if buffer matches pattern at all non-wildcard positions, false otherwise.
     */
    bool matchPattern(const unsigned char* buffer, const std::vector<unsigned char>& pattern);
};

#endif // carve_H