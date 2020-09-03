============================
PoE API software README FILE
============================

PoE API software package contains the followed folders:

1. PoE API software source code
        
    PoE API software source code folder contains the followed items:

    a.  poi_api_projects directory:

         Directory Layout - 

            ./examples/linux/            : example1: example_uart_to_15_bytes_protocol application sources ,
                                                  example2: test_i2c_and_poe_hw application sources.

            ./mscc_arcitecture         : mscc arcitecture sources

            ./mscc_poe_api             : mscc poe api sources

        Note: In order to create the 15 byte protocol conversion module, in the user host, only the two directories - 
                 mscc_arcitecture and mscc_poe_api, need to be compiled. 

    b. PoE API document - 06-0054-056 User Guide PoE API doc Rev_1.0 12.doc

2. PoE Manager Enhanced Mode GUI

	The PoE Manager Enchanced Mode GUI directory is part of the poe api software example.
	The purpose of this GUI is to communicate with the POE API software using the 15 byte protocol.
	This GUI can be used to demonstrate the POE API functionality.
	The GUI need to be installed in a Microsoft windows PC. 
	The example directory:  example_uart_to_15_bytes_protocol demostrate the POE API software by using the POE manager GUI.
	

    a.  PoE manager Enhanced Mode (SS-0050-00N) GUI install file
    b.  POE Manager user guide document: 06-0027-056_PoE_Enhanced_Mode_GUI_UG_v1.3.doc
 

