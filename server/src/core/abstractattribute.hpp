#ifndef ABSTRACTATTRIBUTE_HPP
#define ABSTRACTATTRIBUTE_HPP

#include <string>
#include <enum/attributename.hpp>
#include <enum/valuetype.hpp>

class InterfaceItem;

class AbstractAttribute
{
  protected:
    InterfaceItem& m_item;
    const AttributeName m_name;
    const ValueType m_type;

    void changed();

  public:
    AbstractAttribute(InterfaceItem& item, AttributeName name, ValueType type);

    inline InterfaceItem& item() const { return m_item; }
    inline AttributeName name() const { return m_name; }
    inline ValueType type() const { return m_type; }

    virtual bool toBool() const = 0;
    virtual int64_t toInt64() const = 0;
    virtual double toDouble() const = 0;
    virtual std::string toString() const = 0;
};

#endif
