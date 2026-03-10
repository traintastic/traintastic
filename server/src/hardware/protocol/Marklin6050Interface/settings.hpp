#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_SETTINGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_SETTINGS_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "config.hpp"

namespace Marklin6050 {

class Settings final : public SubObject
{
    CLASS_ID("marklin6050_settings")

private:
    void centralUnitVersionChanged(uint16_t value);

protected:
    void loaded() final;

public:
    Property<uint16_t> centralUnitVersion;
    Property<bool> analog;
    Property<unsigned int> s88amount;
    Property<unsigned int> s88interval;
    Property<unsigned int> turnouttime;
    Property<unsigned int> redundancy;
    Property<bool> extensions;
    //Property<unsigned int> oldAddress;
    //Property<unsigned int> newAddress;
    //Property<bool> programmer;

    Settings(Object& _parent, std::string_view parentPropertyName);

    Config config() const;
    void updateEnabled(bool online);
};

} // namespace Marklin6050

#endif
