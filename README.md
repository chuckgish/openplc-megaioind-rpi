<<<<<<< HEAD
OpenPLC Driver for MEGAIO-IND
=============================

This driver is for the old (original) card from Sequent Microsystems. It uses the blank driver for Linux in the OpenPLC runtime program for the Rasperry Pi.


Usage
-----

1. Backup the existing blank (for Linux) hardware driver

'cd OpenPLC_v3/webserver/core/hardware_layers/

cp blank.cpp blank.cpp_backup'

2. Replace the contents of the blank.cpp file in that directory with the contents of the blank.cpp file in this repository



Physical Addressing
-------------------

|Interface        |Terminals                          | Addressing      |
|-----------------|:---------------------------------:|----------------:|
|Digital In       |Opto-isolated (x4)                 |%IX0.0 - %IX0.3  |
|Digital Out      |Open Drain (x4)                    |%QX0.0 - %QX0.3  |
|                 |Relays (x4)                        |%QX0.4 - %QX0.7  |
|Analog In        |0-10 Volts (x4)                    |%IW0 - %IW3      |
|                 |4-20 mA (x4)                       |%IW4 - %IW7      |
|Analog Out       |0-10 Volts (x4)                    |%QW0 - %QW3      |
|                 |4-20 mA (x4)                       |%QW4 - %QW7      |
=======
# openplc-megaioind-rpi
OpenPLC hardware driver for the original (old model) MegaIO-IND Raspberry Pi expansion card from Sequent Microsystems
>>>>>>> 76e7dde6716688cc0b8b5628e0fc7346fa5cc21e
