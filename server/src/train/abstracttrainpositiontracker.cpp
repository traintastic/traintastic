#include "abstracttrainpositiontracker.hpp"
#include "train.hpp"
#include "../utils/almostzero.hpp"

AbstractTrainPositionTracker::AbstractTrainPositionTracker(const std::shared_ptr<Train> &train)
  : m_train(train)
{

}

AbstractTrainPositionTracker::~AbstractTrainPositionTracker()
{
  if(auto train = m_train.lock())
  {
    train->removeTracker(this);
  }
}

DeadlineTrainPositionTracker::DeadlineTrainPositionTracker(const std::shared_ptr<Train>& train,
                                                           double targetTravelledMeters_,
                                                           const std::function<bool (double &)> &callback)
  : AbstractTrainPositionTracker(train)
  , targetTravelledMeters(targetTravelledMeters_)
  , m_onTargetCallback(callback)
{

}

void DeadlineTrainPositionTracker::trainSpeedChanged(double physicalSpeedMS)
{
  auto now = std::chrono::steady_clock::now();

  if(!almostZero(currentTrainSpeed))
  {
    expectedArrivalTimer.cancel();

    auto elapsed = now - lastSpeedChange;
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

    currentTravelledMeters += currentTrainSpeed * double(millis.count()) / 1000.0;
  }

  currentTrainSpeed = physicalSpeedMS;

  if(currentTravelledMeters > targetTravelledMeters ||
    almostZero(currentTravelledMeters - targetTravelledMeters))
  {
    // We have reached target distance

    const double oldTarget = targetTravelledMeters;

    // Ask if we need to continue
    if(m_onTargetCallback(targetTravelledMeters))
    {
      // New target has been set, reset travelled
      currentTravelledMeters -= oldTarget;
      if(almostZero(currentTravelledMeters))
        currentTravelledMeters = 0;
    }
    else
    {
      // We can stop now
      return;
    }
  }

  if(!almostZero(currentTrainSpeed))
  {
    double remainingMeters = targetTravelledMeters - currentTravelledMeters;
    double remainingSeconds = remainingMeters / currentTrainSpeed;
    lastSpeedChange = now;
    expectedArrivalTimer.expires_after(std::chrono::milliseconds(remainingSeconds * 1000.0));
    expectedArrivalTimer.async_wait(
      [this](const boost::system::error_code& ec)
      {
        if(!ec)
        {
          // Force calculation of current position
          trainSpeedChanged(currentTrainSpeed);
        }
      });
  }
}
