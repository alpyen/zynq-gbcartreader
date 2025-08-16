# PmodA

set_property -dict { PACKAGE_PIN Y18   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[0] }]; # RDn
set_property -dict { PACKAGE_PIN Y19   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[1] }]; # CSn
set_property -dict { PACKAGE_PIN Y16   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[2] }]; # ADDR_SDATA
set_property -dict { PACKAGE_PIN Y17   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[3] }]; # ADDR_RCLK
set_property -dict { PACKAGE_PIN U18   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[4] }]; # WRn
# set_property -dict { PACKAGE_PIN U19   IOSTANDARD LVCMOS33 } [get_ports { ja[5] }]; # x
set_property -dict { PACKAGE_PIN W18   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[5] }]; # ADDR_SCLK
# set_property -dict { PACKAGE_PIN W19   IOSTANDARD LVCMOS33 } [get_ports { ja[7] }]; # x

# PmodB

set_property -dict { PACKAGE_PIN W14   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[6] }]; # DATA_OUT_SDATA
set_property -dict { PACKAGE_PIN Y14   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[7] }]; # DATA_OUT_RCLK
set_property -dict { PACKAGE_PIN T11   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[8] }]; # DATA_IN_SDATA
set_property -dict { PACKAGE_PIN T10   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[9] }]; # DATA_IN_RCLK
set_property -dict { PACKAGE_PIN V16   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[10] }]; # DATA_OUT_OE
set_property -dict { PACKAGE_PIN W16   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[11] }]; # DATA_OUT_SCLK
set_property -dict { PACKAGE_PIN V12   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[12] }]; # DATA_IN_PL
set_property -dict { PACKAGE_PIN W13   IOSTANDARD LVCMOS33 } [get_ports { pmods_tri_io[13] }]; # DATA_IN_SCLK
