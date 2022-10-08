#include "lncvprogrammingcontroller.hpp"
#include "lncvprogrammer.hpp"
#include "../../../core/idobject.hpp"
#include "../../../world/world.hpp"

bool LNCVProgrammingController::lncvAttach(LNCVProgrammer& programmer)
{
  if(m_programmer)
    return false;

  m_programmer = &programmer;
  return true;
}

void LNCVProgrammingController::lncvDetach(LNCVProgrammer& programmer)
{
  if(m_programmer == &programmer)
  {
    stopLNCVProgramming();
    m_programmer = nullptr;
  }
}

void LNCVProgrammingController::addToWorld()
{
  if(auto* object = dynamic_cast<IdObject*>(this))
    object->world().lncvProgrammingControllers->add(std::dynamic_pointer_cast<LNCVProgrammingController>(object->shared_from_this()));
  else
    assert(false);
}

void LNCVProgrammingController::destroying()
{
  if(auto* object = dynamic_cast<IdObject*>(this))
    object->world().lncvProgrammingControllers->remove(std::dynamic_pointer_cast<LNCVProgrammingController>(object->shared_from_this()));
  else
    assert(false);

  if(m_programmer)
    m_programmer->destroy();
}
