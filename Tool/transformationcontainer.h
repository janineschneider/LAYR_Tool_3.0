#ifndef TRANSFORMATIONCONTAINER_H
#define TRANSFORMATIONCONTAINER_H

#include "bytecontainer.h"
#include <memory>

/**
 * \brief ByteContainer backed by an in-memory byte vector, holding data produced by a
 *        transformation rule. Owns its data independently of any external stream.
 */

class TransformationContainer : public ByteContainer
{
public:
    /// \brief Construct from the byte vector produced by a transformation rule
    explicit TransformationContainer(std::vector<unsigned char> data);

    const uint64_t size() const override;
    unsigned char at(uint64_t position, uint64_t lowerBound, uint64_t upperBound) override;
    unsigned char at(uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound) override;
    unsigned char at(uint64_t position) const override;
    unsigned char front() override;
    unsigned char back() override;
    std::vector<unsigned char> copy2Vector(uint64_t start, uint64_t end) override;
    void print2Cout(uint64_t start, uint64_t end) override;

private:
    /// \brief Shared, immutable ownership of the transformed byte data
    std::shared_ptr<const std::vector<unsigned char>> m_data;
};

#endif // TRANSFORMATIONCONTAINER_H