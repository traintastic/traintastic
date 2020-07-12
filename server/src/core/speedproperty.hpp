

#include "unitproperty.hpp"
#include "../enum/speedunit.hpp"

using SpeedProperty = UnitProperty<double, SpeedUnit>;


double getValue(SpeedUnit unit) const;
void setValue(double value, SpeedUnit unit):
SpeedUnit unit() const;
void setUnit(SpeedUnit value)
{
  setValue(getValue(value), value);
}
