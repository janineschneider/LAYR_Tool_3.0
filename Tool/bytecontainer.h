#ifndef BYTECONTAINER_H
#define BYTECONTAINER_H

#include <cstdint>
#include <istream>
#include <vector>

/**
 * \brief Abstract interface for byte-addressable data sources. Implemented by RootContainer
 *        (stream-backed, original input) and TransformationContainer (vector-backed, output
 *        of a transformation rule), letting rules read either uniformly.
 */
class ByteContainer
{
public:
  virtual ~ByteContainer() = default;

  /**
    * \brief Function which returns the size of the underlying data
    * \return Size as const size_t (Byte)
    */
  virtual const uint64_t size() const = 0;

  /**
   * \brief Character access via index
   * \param position Index as int (Byte)
   * \param lowerBound Lower block sequence access bound (Byte)
   * \param upperBound Upper block sequence access bound (Byte)
   * \return Reference to unsigned char
   */
  virtual unsigned char at(uint64_t position, uint64_t lowerBound, uint64_t upperBound) = 0;

  /**
   * \brief Character access via index and pointer offset
   * \param offset Offset as int (Byte)
   * \param position Index as int (Byte)
   * \param lowerBound Lower block sequence access bound (Byte)
   * \param upperBound Upper block sequence access bound (Byte)
   * \return Reference to unsigned char
   */
  virtual unsigned char at(uint64_t offset, uint64_t position, uint64_t lowerBound, uint64_t upperBound) = 0;

  /**
   * \brief Returns the first character of the underlying data
   * \return Reference to unsigned char
   */
  virtual unsigned char front() = 0;

  /**
   * \brief Returns the last character of the underlying data
   * \return Reference to unsigned char
   */
  virtual unsigned char back() = 0;

  /**
   * \warning Do not copy big sequences with that function!
   * \brief Copies a data sequence to a vector \n
   *        Can be used to get a sequence of bytes from the underlying data \n
   *        For small content pieces (like file allocation table) ONLY
   * \param start Starting index of byte sequence (Byte)
   * \param end Ending index of byte sequence (Byte)
   * \return Content of block sequence as bytes
   */
  virtual std::vector<unsigned char> copy2Vector(uint64_t start, uint64_t end) = 0;

  /**
   * \brief Function to print content of the underlying data to cout
   * \param start Begin of the content which shall be printed (Byte)
   * \param end End of the content which shall be printed (Byte)
   */
  virtual void print2Cout(uint64_t start, uint64_t end) = 0;

  /**
   * \brief Character access via index
   * \param position Index as int (Byte)
   * \return Reference to unsigned char
   */
  virtual unsigned char at(uint64_t position) const = 0;
};

#endif // BYTECONTAINER_H
