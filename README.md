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
|Digital Out      |Open Drain (x4) - Relays (x4)      |%QX0.0 - %QX0.7  |
