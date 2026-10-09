#ifndef ROOTCONTAINER_H
#define ROOTCONTAINER_H

#include "bytecontainer.h"
#include <istream>

/**
 * @brief ByteContainer backed by an external input stream (e.g. an opened disk image).
 *        Represents the original, un-transformed input at the root of the address tree.
 */
class RootContainer : public ByteContainer
{
public:
    /// \brief Construct from an input stream containing the raw data to read
    explicit RootContainer(std::istream& input);

    const uint64_t size() const override;
    unsigned char at(uint64_t position, uint64_t lowerBound, uint64_t upperBound) override;
    unsigned char at(uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound) override;
    unsigned char at(uint64_t position) const override;
    unsigned char front() override;
    unsigned char back() override;
    std::vector<unsigned char> copy2Vector(uint64_t start, uint64_t end) override;
    void print2Cout(uint64_t start, uint64_t end) override;

private:
    /// \brief Externally owned stream; caller must ensure it outlives this container
    std::istream& m_input;
};

#endif // ROOTCONTAINER_H