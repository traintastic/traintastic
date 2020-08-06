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




# RailVehicle
- id
- name
- lob
- weight
- decoder
- decoder2


# train
- trothlle speed
- weight simulation: 0=off, 1=real, 2=half

# netwerk
enum als string over netwerk?? nummers zijn dan alleen intern, niet in json/netwerk

# controller usb xpressnet interface
- allow estop
- allow power off
- mode:
  - direct: multimaus decoder addr == decoder addr
  - virtual decoder: multimaus decoder addr = assigend decoder addr
  - virtual maus: multimaus addr = assigend decoder addr


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


# usb xpressnet

adm2484 iso rs485




