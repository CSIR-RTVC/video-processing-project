#pragma once

#include <sstream>

template <class T>
T convert(const std::string& sStringValue ) 
{
  T val;
  std::istringstream istr(sStringValue);
  istr >> val;
  return val;
}

template <class T>
std::string toString(T t)
{
  std::ostringstream ostr;
  ostr << t;
  return ostr.str();
}
