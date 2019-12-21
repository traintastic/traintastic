#ifndef INTERFACEITEMATTRIBUTE_HPP
#define INTERFACEITEMATTRIBUTE_HPP

#include "abstractattribute.hpp"
#include "to.hpp"
#include "valuetypetraits.hpp"

template<typename T>
class Attribute : public AbstractAttribute
{
  protected:
    T m_value;

  public:
    Attribute(InterfaceItem& item, AttributeName name, const T& value) :
      AbstractAttribute{item, name, value_type_v<T>},
      m_value{value}
    {
    }

    bool toBool() const final
    {
      return to<bool>(m_value);
    }

    int64_t toInt64() const final
    {
      return to<int64_t>(m_value);
    }

    double toDouble() const final
    {
      return to<double>(m_value);
    }

    std::string toString() const final
    {
      return to<std::string>(m_value);
    }

    void setValue(const T& value)
    {
      if(m_value != value)
      {
        m_value = value;
        changed();
      }
    }
};

#endif
