#include "abstractattribute.hpp"
#include "interfaceitem.hpp"
#include "object.hpp"

AbstractAttribute::AbstractAttribute(InterfaceItem& item, AttributeName name, ValueType type) :
  m_item{item},
  m_name{name},
  m_type{type}
{
}

void AbstractAttribute::changed()
{
  m_item.object().attributeChanged(*this);
}
