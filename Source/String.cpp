//
#include "Recluse/String.hpp"

namespace Recluse {


String::String(size_t count) 
    : m_size(count)
    , m_cStr(nullptr) 
{ 
    if (count) m_cStr = new U8[count]; 
}


String::~String()
{
    if (m_cStr)
    {
        delete[] m_cStr;
        m_cStr = nullptr;
    }

    m_size = 0;
}


std::string toLowerCase(const std::string& str)
{
    std::string lowerStr;
    lowerStr.resize(str.size());
    for (U32 i = 0; i < lowerStr.size(); ++i)
    {
        lowerStr[i] = std::tolower(str[i]);
    }
    return lowerStr;
}


std::string toUpperCase(const std::string& str)
{
    std::string upperStr;
    upperStr.resize(str.size());
    for (U32 i = 0; i < upperStr.size(); ++i)
    {
        upperStr[i] = std::toupper(str[i]);
    }
    return upperStr;
}
} // Recluse