#ifndef CUSTOMEXCEPTION_H
#define CUSTOMEXCEPTION_H

#include <exception>

/**
 * \brief Custom exception class \n
 *        Exception for input sequence access border violation
 */
class border_violation : public std::exception
{
  virtual const char* what() const throw()
  {
    return "Violation of lower or upper block sequence input border";
  }
};

/// \brief Shared, single instance of border_violation thrown by ByteContainer bound checks
inline border_violation bvex;

#endif // CUSTOMEXCEPTION_H