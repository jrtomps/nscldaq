
#include <CDataSinkException.h>


CDataSinkException::CDataSinkException(std::string reason)
  : CException("DataSink operation"),
  m_reason(reason)
{}

CDataSinkException::CDataSinkException(std::string context, std::string reason)
  : CException(context),
  m_reason(reason)
{}


const char* CDataSinkException::ReasonText() const
{
  return m_reason.c_str();
}
