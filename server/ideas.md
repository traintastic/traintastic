# length/speed

LengthProperty
- double value
- enum unit: mm, cm, m, inch, ft, yard

SpeedProperty
- double value
- enum speed: m/s, km/h, mph

WeightProperty
- double value
- enum unit: T (metric)






# task thread

  std::thread task(
    [loconet=shared_ptr<LocoNet>()]()
    {
      for(uint8_t slot = SLOT_LOCO_MIN; slot <= SLOT_LOCO_MAX; slot++)
      {
        EventLoop::call(
          [loconet, slot]()
          {
            loconet->send(RequestSlotData(slot));
          });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    });
  task.detach();






