# Wireless Geiger Counter
A distributed radiation detection networks requires a low power, rugged, portable, and wireless detection modules. These prototypes are housed in a plastic shell allowing them to be made waterproof, transmit data over Wi-Fi allowing them to be placed anywhere you can host a network, and are designed to last for months from a single charge. A Geiger tube provides the signal that is read in by a microcontroller that then transmits the counts to a database.

![][assembledTube]

## Hardware
A single board that hosts the electronics required to take power from a lithium ion cell, provide the bias voltage required by the Geiger tube (new old stock Russian SBM-20), and transmit that data over Wi-Fi.

![][labeledSections]

### Power
The power to the module is provided through a 18650 Li-ion chemistry cell. This is a rechargeable cell that is very common (used in laptop batteries, Tesla vehicles, and LED flashlights) with a capacity of 1.5Ah - 3.4Ah. The battery has a management circuit that prevents over/under voltage and allows charging. It also provides fuel gauging to the microcontroller. The rest of the board is supplied a regulated 3.3V.

### Wireless
The espressif module has a on board antenna although a chip antenna can be substituted. The Wi-Fi using the on board antennas on a ESP-12F module is shown to have a range of at least 250m with almost a km under ideal conditions. It is able to connect on 802.11 b/g/n with a maximum data rate of 150Mbps.

### Microcontroller
The ESP8266/ESP8285 has a integrated Tensilica L106 Diamond series 32-bit processor running at 80Mhz. The 12E/F module breaks out the 13 GPIO pins and one 10-bit ADC. Some of those pins are reserved for communication to storage, and wake from deep sleep. The pins are not 5V tolerant and average current consumption, including connection to Wi-Fi is 80mA, though peaks currents approach 500mA for short (<5ms) bursts.

### High voltage
The high voltage section is a Schmitt-trigger oscillator based boost converter feeding a Cockcroft-Walton voltage multiplier with the output divided and taken to a op-amp comparing against a voltage reference. The previous design used the microcontroller to boost the voltage, but that resulted in a 80mA constant current draw as the microcontroller could not be taken to deep sleep with the PWM running. The design was adapted from the following [application note](https://www.maximintegrated.com/en/app-notes/index.mvp/id/3757) form Maxim.

![][oldPrototypes]

## Software
The software only resides on the microcontroller, and can be found [here](https://github.com/Sawaiz/geigerControl). The other required part is a SQL database to transmit the information to.


[oldPrototypes]: cad/renderings/oldPrototypes.jpg "Older prototypes"
[assembledTube]: cad/renderings/assembledTube.png "Fully Assembled Tube"
[labeledSections]: cad/renderings/labeledSections.png "Assembled Tube"
