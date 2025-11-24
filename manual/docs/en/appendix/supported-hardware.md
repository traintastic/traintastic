# Supported hardware

This page lists command stations and digital systems that have been tested with Traintastic.
Using the setup wizard is **recommended** whenever possible.
If wizard support is not available, setup can still be done manually by creating and configuring the correct interface type.

For details on manual interface configuration, see the [Interface configuration](../advanced/interface/index.md) section.

!!! tip "Not on the list?"
    Your system may still work even if it isn’t listed here.
    Visit the [community forum](https://discourse.traintastic.org) to see if others have tried it, or share your own experience to help extend this list.

## Supported command stations

| Vendor / System       | Interface type(s)             | Notes                           |
|-----------------------|-------------------------------|---------------------------------|
| DCC-EX                | USB, Network                  | -                               |
| Digikeijs DR5000      | USB, Network                  | -                               |
| ESU ECoS              | Network                       | -                               |
| ESU ECoS 2            | Network                       | -                               |
| LDT HSI-88            | Serial                        | No wizard support.              |
| Märklin CS2           | Network                       | -                               |
| Roco MultiMAUS        | XpressNet                     | Requires a XpressNet interface. |
| Uhlenbrock Intellibox | LocoNet, Serial not supported | Requires a LocoNet interface.   |
| Uhlenbrock IB-COM     | USB                           | -                               |

!!! note
    All systems listed here have been verified either with the setup wizard or through manual configuration.
    If you encounter issues with a listed system, please report them on the [community forum](https://discourse.traintastic.org) so the documentation and wizard can be improved.

## Untested command stations

These systems are expected to work with Traintastic, but have not yet been verified.

| Vendor / System             | Interface type(s)             | Wizard support | Notes                                   |
|-----------------------------|-------------------------------|----------------|-----------------------------------------|
| Digitrax LocoNet            | Serial, USB, Network          | No             | Configure LocoNet interface manually.   |
| Lenz XpressNet              | Serial, USB, Network          | No             | Configure XpressNet interface manually. |
| Märklin CS3                 | Network                       | Yes            | -                                       |
| Märklin CS3 plus            | Network                       | Yes            | -                                       |
| Uhlenbrock Intellibox Basic | USB                           | Yes            | -                                       |
| Uhlenbrock Intellibox IR    | LocoNet, Serial not supported | Yes            | Requires a LocoNet interface.           |
| Uhlenbrock Intellibox II    | USB                           | Yes            | -                                       |
| Uhlenbrock Intellibox 2neo  | USB                           | Yes            | -                                       |
| YaMoRC YD7001               | USB, Network                  | No             | Similar to Digikeijs DR5000.            |
| YaMoRC YD7010               | USB, Network                  | No             | Similar to Digikeijs DR5000.            |
| z21 start                   | Network                       | Yes            | -                                       |
| z21 white                   | Network                       | Yes            | -                                       |
| Z21 black                   | Network                       | Yes            | -                                       |

!!! tip "Share your success!"
    Have you successfully used one of these systems with Traintastic?
    Please share your experience on the [community forum](https://discourse.traintastic.org).
    Your feedback helps verify compatibility and improve this list for other users.

## Unsupported command stations

These systems are currently **not supported**.
Some lack sufficient technical documentation for implementation.
Supported for others has not been developed due to missing hardware.

| Vendor / System | Interface Type(s) | Notes               |
|-----------------|-------------------|---------------------|
| BiDiB           | USB, Network      | -                   |
| Selectix        | Serial            | In development      |
| VPEB Dinamo     | Serial, USB       | Track driver system |

!!! tip "Contributions are welcome!"
    Can you help improve support for these systems?
    Testing results, feedback, or legally obtained technical documentation are very welcome.
    Please share on the [community forum](https://discourse.traintastic.org).
