# ZYNQ GBCartReader - Read & Write GameBoy cartridges

This project is aimed to dive into FPGA SoCs and embedded software development
by desining a system that's able to read and write to gameboy cartridges with
a custom designed PCB which interfaces between the FPGA and the game cartridge.

Starting from a very basic bitbanging implementation solely running on the ARM core,
the following design iterations will make use of a more sophisticated architecture
by offloading the board communication to an IP core and moving commands and responses
around with AXI interfaces.
