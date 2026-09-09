#include "transformationcontainer.h"
#include "customexception.h"

#include <iostream>

TransformationContainer::TransformationContainer(std::vector<unsigned char> data)
    : m_data(std::make_shared<const std::vector<unsigned char>>(std::move(data)))
{
}

const uint64_t TransformationContainer::size() const
{
    return m_data->size();
}

unsigned char TransformationContainer::at(uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    if ((position < lowerBound) || (position > upperBound)) {
        throw bvex;
    }
    return at(position);
}

unsigned char TransformationContainer::at(uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    if (((offset + position) < lowerBound) || ((offset + position) > upperBound)) {
        throw bvex;
    }
    return at(offset + position);
}

unsigned char TransformationContainer::at(uint64_t position) const
{
    return m_data->at(position);
}

unsigned char TransformationContainer::front() { return at(0); }

unsigned char TransformationContainer::back() { return m_data->back(); }

std::vector<unsigned char> TransformationContainer::copy2Vector(uint64_t start, uint64_t end)
{
    if (end >= m_data->size()) end = m_data->size() - 1;
    return std::vector<unsigned char>(m_data->begin() + start, m_data->begin() + end + 1);
}

void TransformationContainer::print2Cout(uint64_t start, uint64_t end)
{
    for (uint64_t position = start; position < end; ++position) {
        std::cout.put(static_cast<char>(m_data->at(position)));
    }
}