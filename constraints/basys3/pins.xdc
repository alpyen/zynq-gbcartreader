# Pmod Header JB

set_property -dict { PACKAGE_PIN A14   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[0] }]; # RDn
set_property -dict { PACKAGE_PIN A16   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[1] }]; # CSn
set_property -dict { PACKAGE_PIN B15   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[2] }]; # ADDR_SDATA
set_property -dict { PACKAGE_PIN B16   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[3] }]; # ADDR_RCLK
set_property -dict { PACKAGE_PIN A15   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[4] }]; # WRn
# set_property -dict { PACKAGE_PIN A17   IOSTANDARD LVCMOS33 } [get_ports { ja[5] }]; # x
set_property -dict { PACKAGE_PIN C15   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[5] }]; # ADDR_SCLK
# set_property -dict { PACKAGE_PIN C16   IOSTANDARD LVCMOS33 } [get_ports { ja[7] }]; # x

# Pmod Header JC

set_property -dict { PACKAGE_PIN K17   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[6] }]; # DATA_OUT_SDATA
set_property -dict { PACKAGE_PIN M18   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[7] }]; # DATA_OUT_RCLK
set_property -dict { PACKAGE_PIN N17   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[8] }]; # DATA_IN_SDATA
set_property -dict { PACKAGE_PIN P18   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[9] }]; # DATA_IN_RCLK
set_property -dict { PACKAGE_PIN L17   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[10] }]; # DATA_OUT_OE
set_property -dict { PACKAGE_PIN M19   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[11] }]; # DATA_OUT_SCLK
set_property -dict { PACKAGE_PIN P17   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[12] }]; # DATA_IN_PL
set_property -dict { PACKAGE_PIN R18   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[13] }]; # DATA_IN_SCLK
