# Clock signal 125 MHz

# The design gets its PL clock from the PS which is driven directly by the 50 MHz oscillator
# and automatically constrained by the block design.

# set_property -dict { PACKAGE_PIN H16   IOSTANDARD LVCMOS33 } [get_ports { sysclk }]; #IO_L13P_T2_MRCC_35 Sch=sysclk
# create_clock -add -name sys_clk_pin -period 8.00 -waveform {0 4} [get_ports { sysclk }];
