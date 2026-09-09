#include "rootcontainer.h"
#include "customexception.h"

#include <iostream>

RootContainer::RootContainer(std::istream& input) : m_input(input) {}

const uint64_t RootContainer::size() const
{
    m_input.seekg(0, m_input.end);
    size_t length = m_input.tellg();
    m_input.seekg(0, m_input.beg);
    return length;
}

unsigned char RootContainer::at(uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    if ((position < lowerBound) || (position > upperBound)) {
        throw bvex;
    }
    return at(position);
}

unsigned char RootContainer::at(uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound)
{
    if (((offset + position) < lowerBound) || ((offset + position) > upperBound)) {
        throw bvex;
    }
    return at(offset + position);
}

unsigned char RootContainer::at(uint64_t position) const
{
    m_input.seekg(position);
    unsigned char singleByte = (unsigned char)m_input.get();
    m_input.seekg(0, m_input.beg);
    m_input.clear();
    return singleByte;
}

unsigned char RootContainer::front() { return at(0); }

unsigned char RootContainer::back()
{
    size_t last = size() - 1;
    m_input.seekg(last);
    unsigned char singleByte = (unsigned char)m_input.get();
    m_input.seekg(0, m_input.beg);
    return singleByte;
}

std::vector<unsigned char> RootContainer::copy2Vector(uint64_t start, uint64_t end)
{
    uint64_t size = end - start + 1;
    uint64_t counter = 0;
    std::vector<unsigned char> sequence;

    m_input.seekg(start);
    char c;
    while (m_input.get(c)) {
        if (counter >= size) break;
        sequence.push_back((unsigned char)c);
        counter++;
    }
    m_input.seekg(0, m_input.beg);
    return sequence;
}

void RootContainer::print2Cout(uint64_t start, uint64_t end)
{
    uint64_t count = (end - start);
    uint32_t bufferSize = 1048576;
    char buffer[1048576];

    if (count < bufferSize) bufferSize = count;

    while (count >= bufferSize) {
        m_input.seekg(start);
        m_input.read(buffer, bufferSize);
        std::cout.write(buffer, bufferSize);
        count -= bufferSize;
        start += bufferSize;
    }

    m_input.read(buffer, count);
    std::cout.write(buffer, count);
    m_input.seekg(0, m_input.beg);
}

