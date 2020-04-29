#include "z21.hpp"
#include "../decoder/decoder.hpp"

//namespace Protocol::Z21 {

z21_lan_x_loco_info::z21_lan_x_loco_info(const Hardware::Decoder& decoder) :
  z21_lan_x_loco_info()
{
  setAddress(decoder.address, decoder.longAddress);
  setSpeedSteps(decoder.speedSteps);
  setDirection(decoder.direction);
  if(decoder.emergencyStop)
    setEmergencyStop();
  else
    setSpeedStep(decoder.speedStep);
  for(auto function : *decoder.functions)
    setFunction(function->number, function->value);
  calcChecksum();
}

//}
