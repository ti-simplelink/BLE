ble_energia
===========

This repository hosts the active development for TI's BLE Energia solution. ble_energia adds Energia support to the CC2650 in network processor configuration, using the MSP432 as the application processor.

All documentation is hosted on [BLE Energia Page](http://energia.nu/reference/BLE/). Please visit this page for more information

**Note: This repo hosts the bleeding edge versions of ble_energia, stable releases are bundled within the Energia IDE's board manager package for MSP432**

Required Hardware and Software
==============================

* The software in this repo
* [Energia 18 (1.6.10E18)](http://energia.nu/download/)
* [MSP432P401R LaunchPad](http://www.ti.com/tool/msp-exp432p401r)
* CC2650 [BoosterPack](http://www.ti.com/tool/boostxl-cc2650ma) running the Simple Network Processor Image
 - The sketches currently build with power savings. You can get SNP images from [this link](http://software-dl.ti.com/dsps/forms/self_cert_export.html?prod_no=ble_2_02_simple_np_setup.exe&ref_url=http://software-dl.ti.com/lprf/BLE-Simple-Network-Processor-Hex-Files).
 - For CC2650MOD BoosterPack you will want to use `simple_np_cc2650bp_uart_pm_xsbl.hex`

Steps to setup
==============

1. Update to the latest Network Processor image, more on this below
2. Clone or download this repository
3. Install MSP432 Board files
 * Tools -> Board -> Board Manager
4. Symlink/Copy to this repo
 * Navigate to <ENERGIA_PATH>/energia-1.6.10E18/libraries
 * Copy the /BLE folder from this repo into the libraries folder above
5. Re-start Energia, BLE examples should now appear in the library
