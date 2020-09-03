/**************************************************************
*
* Lattice Semiconductor Corp. Copyright 2008
* 
* ispVME Embedded allows programming of Lattice's suite of FPGA
* devices on embedded systems through the JTAG port.  The software
* is distributed in source code form and is open to re - distribution
* and modification where applicable.
*
* ispVME Embedded C Source comprised with 3 modules:
* ispvm_ui.c is the module provides input and output support.
* ivm_core.c is the module interpret the VME file(s).
* hardware.c is the module access the JTAG port of the device(s).                 
*
* The optional module cable.c is for supporting Lattice's parallel 
* port ispDOWNLOAD cable on DOS and Windows 95/98 O/S. It can be 
* requested from Lattice's ispVMSupport.
*
***************************************************************/


/**************************************************************
* 
* Revision History of ispvm_ui.c
* 
* 3/6/07 ht Added functions vme_out_char(),vme_out_hex(), 
*           vme_out_string() to provide output resources.
*           Consolidate all BACKDOOR_MGR_Printf() calls into the added output 
*           functions.	
*
* 09/11/07 NN Added Global variables initialization
* 09/24/07 NN Added a switch allowing users to do calibration.
* Calibration will help to determine the system clock frequency
* and the count value for one micro-second delay of the target 
* specific hardware.
* Removed Delay Percent support
* 11/15/07  NN moved the checking of the File CRC to the end of processing
***************************************************************/


#include "vmopcode.h"

/***************************************************************
*
* File pointer to the VME file.
*
***************************************************************/

#define NULL	0

//FILE * g_pVMEFile = NULL;
unsigned char *g_pVMEBuf = NULL;
unsigned long g_pVMESize = 0; 


extern int isp_execute(unsigned char *buf, unsigned long bufsize, int check);

/***************************************************************
*
* Functions declared in this ispvm_ui.c module
*
***************************************************************/
unsigned char GetByte(void);
void vme_out_char(unsigned char charOut);
void vme_out_hex(unsigned char hexOut);
void vme_out_string(char *stringOut);
void ispVMMemManager( signed char cTarget, unsigned short usSize );
void ispVMFreeMem(void);
void error_handler( short a_siRetCode, char * pszMessage );
signed char ispVM( unsigned char *buf, unsigned long bufsize );

/***************************************************************
*
* Global variables.
*
***************************************************************/
unsigned short g_usPreviousSize = 0;
unsigned short g_usExpectedCRC = 0;

/***************************************************************
*
* External variables and functions declared in ivm_core.c module.
*
***************************************************************/
extern signed char ispVMCode();
extern void ispVMCalculateCRC32( unsigned char a_ucData );
extern void ispVMStart();
extern void ispVMEnd();
extern unsigned short g_usCalculatedCRC;
extern unsigned short g_usDataType;
extern unsigned char * g_pucOutMaskData,
                     * g_pucInData,
					 * g_pucOutData,
					 * g_pucHIRData,
					 * g_pucTIRData,
					 * g_pucHDRData,
					 * g_pucTDRData,
					 * g_pucOutDMaskData,
                     * g_pucIntelBuffer;
extern unsigned char * g_pucHeapMemory;
extern unsigned short g_iHeapCounter;
extern unsigned short g_iHEAPSize;
extern unsigned short g_usIntelDataIndex;
extern unsigned short g_usIntelBufferSize;
extern LVDSPair * g_pLVDSList;

/***************************************************************
*
* External variables and functions declared in hardware.c module.
*
***************************************************************/
extern short int calibration(void);
extern unsigned short g_usCpu_Frequency;

/***************************************************************
*
* Supported VME versions.
*
***************************************************************/

const char * const g_szSupportedVersions[] = { "__VME2.0", "__VME3.0", "____12.0", "____12.1", 0 };

/***************************************************************
*
* GetByte
*
* Returns a byte to the caller. The returned byte depends on the
* g_usDataType register. If the HEAP_IN bit is set, then the byte
* is returned from the HEAP. If the LHEAP_IN bit is set, then
* the byte is returned from the intelligent buffer. Otherwise,
* the byte is returned directly from the VME file.
*
***************************************************************/

unsigned char GetByte()
{
	unsigned char ucData = 0;
	
	if ( g_usDataType & HEAP_IN ) {

		/***************************************************************
		*
		* Get data from repeat buffer.
		*
		***************************************************************/

		if ( g_iHeapCounter > g_iHEAPSize ) {

			/***************************************************************
			*
			* Data over-run.
			*
			***************************************************************/

			return 0xFF;
		}

		ucData = g_pucHeapMemory[ g_iHeapCounter++ ];
	}
	else if ( g_usDataType & LHEAP_IN ) {

		/***************************************************************
		*
		* Get data from intel buffer.
		*
		***************************************************************/

		if ( g_usIntelDataIndex >= g_usIntelBufferSize ) {

			/***************************************************************
			*
			* Data over-run.
			*
			***************************************************************/

			return 0xFF;
		}

		ucData = g_pucIntelBuffer[ g_usIntelDataIndex++ ];
	}
	else {

		/***************************************************************
		*
		* Get data from file.
		*
		***************************************************************/

//		ucData = (unsigned char)fgetc( g_pVMEFile );
        if( --g_pVMESize >= 0 )
            ucData = *g_pVMEBuf++;
        else
//		if ( feof( g_pVMEFile ) ) 
        {

			/***************************************************************
			*
			* Reached EOF.
			*
			***************************************************************/

			return 0xFF;
		}
		/***************************************************************
		*
		* Calculate the 32-bit CRC if the expected CRC exist.
		*
		***************************************************************/
		if( g_usExpectedCRC != 0)
		{
			ispVMCalculateCRC32(ucData);
		}
	}
	
	return ( ucData );
}

/***************************************************************
*
* vme_out_char
*
* Send a character out to the output resource if available. 
* The monitor is the default output resource. 
*
*
***************************************************************/
void vme_out_char(unsigned char charOut)
{
	BACKDOOR_MGR_Printf("%c",charOut);
}
/***************************************************************
*
* vme_out_hex
*
* Send a character out as in hex format to the output resource 
* if available. The monitor is the default output resource. 
*
*
***************************************************************/
void vme_out_hex(unsigned char hexOut)
{
	BACKDOOR_MGR_Printf("%.2X",hexOut);
}
/***************************************************************
*
* vme_out_string
*
* Send a text string out to the output resource if available. 
* The monitor is the default output resource. 
*
*
***************************************************************/
void vme_out_string(char *stringOut)
{
	if(stringOut)
	{
		BACKDOOR_MGR_Printf("%s",stringOut);
	}

}
/***************************************************************
*
* ispVMMemManager
*
* Allocate memory based on cTarget. The memory size is specified
* by usSize.
*
***************************************************************/

void ispVMMemManager( signed char cTarget, unsigned short usSize )
{
	switch ( cTarget ) {
	case XTDI:
    case TDI:  
		if ( g_pucInData != NULL ) {
			if ( g_usPreviousSize == usSize ) {/*memory exist*/
				break;
			}
			else {
				free( g_pucInData );
				g_pucInData = NULL;
			}
		}
		g_pucInData = ( unsigned char * ) malloc( usSize / 8 + 2 );
		g_usPreviousSize = usSize;
    case XTDO:
    case TDO:
		if ( g_pucOutData!= NULL ) { 
			if ( g_usPreviousSize == usSize ) { /*already exist*/
				break;
			}
			else {
				free( g_pucOutData );
				g_pucOutData = NULL;
			}
		}
		g_pucOutData = ( unsigned char * ) malloc( usSize / 8 + 2 );
		g_usPreviousSize = usSize;
		break;
    case MASK:
		if ( g_pucOutMaskData != NULL ) {
			if ( g_usPreviousSize == usSize ) {/*already allocated*/
				break;
			}
			else {
				free( g_pucOutMaskData ); 
				g_pucOutMaskData = NULL;
			}
		}
		g_pucOutMaskData = ( unsigned char * ) malloc( usSize / 8 + 2 );
		g_usPreviousSize = usSize;
		break;
    case HIR:
		if ( g_pucHIRData != NULL ) {
			free( g_pucHIRData );
			g_pucHIRData = NULL;
		}
		g_pucHIRData = ( unsigned char * ) malloc( usSize / 8 + 2 );
		break;
    case TIR:
		if ( g_pucTIRData != NULL ) {
			free( g_pucTIRData );
			g_pucTIRData = NULL;
		}
		g_pucTIRData = ( unsigned char * ) malloc( usSize / 8 + 2 );
		break;
    case HDR:
		if ( g_pucHDRData != NULL ) {
			free( g_pucHDRData );
			g_pucHDRData = NULL;
		}
		g_pucHDRData = ( unsigned char * ) malloc( usSize / 8 + 2 );
		break;
    case TDR:
		if ( g_pucTDRData != NULL ) {
			free( g_pucTDRData );
			g_pucTDRData = NULL;
		}
		g_pucTDRData = ( unsigned char * ) malloc( usSize / 8 + 2 );
		break;
    case HEAP:
		if ( g_pucHeapMemory != NULL ) {
			free( g_pucHeapMemory );
			g_pucHeapMemory = NULL;
		}
		g_pucHeapMemory = ( unsigned char * ) malloc( usSize + 2 );
		break;
	case DMASK: 
		if ( g_pucOutDMaskData != NULL ) {
			if ( g_usPreviousSize == usSize ) { /*already allocated*/
				break;
			}
			else {
				free( g_pucOutDMaskData ); 
				g_pucOutDMaskData = NULL;
			}
		}
		g_pucOutDMaskData = ( unsigned char * ) malloc( usSize / 8 + 2 );
		g_usPreviousSize = usSize;
		break;
	case LHEAP:
		if ( g_pucIntelBuffer != NULL ) {
			free( g_pucIntelBuffer );
			g_pucIntelBuffer = NULL;
		}
		g_pucIntelBuffer = ( unsigned char * ) malloc( usSize + 2 );
		break;
	case LVDS:
		if ( g_pLVDSList != NULL ) {
			free( g_pLVDSList );
			g_pLVDSList = NULL;
		}
		g_pLVDSList = ( LVDSPair * ) calloc( usSize, sizeof( LVDSPair ) );
		break;
	default:
		return;
    }
}

/***************************************************************
*
* ispVMFreeMem
*
* Free memory that were dynamically allocated.
*
***************************************************************/

void ispVMFreeMem()
{
	if ( g_pucHeapMemory != NULL ) {
		free( g_pucHeapMemory ); 
		g_pucHeapMemory = NULL;
	}

	if ( g_pucOutMaskData != NULL ) {
		free( g_pucOutMaskData );
		g_pucOutMaskData = NULL;
	}
	
	if ( g_pucInData != NULL ) {
		free( g_pucInData );
		g_pucInData = NULL;
	}
	
	if ( g_pucOutData != NULL ) {
		free( g_pucOutData );
		g_pucOutData = NULL;
	}
	
	if ( g_pucHIRData != NULL ) {
		free( g_pucHIRData );
		g_pucHIRData = NULL;
	}
	
	if ( g_pucTIRData != NULL ) {
		free( g_pucTIRData );
		g_pucTIRData = NULL;
	}
	
	if ( g_pucHDRData != NULL ) {
		free( g_pucHDRData );
		g_pucHDRData = NULL;
	}
	
	if ( g_pucTDRData != NULL ) {
		free( g_pucTDRData );
		g_pucTDRData = NULL;
	}
	
	if ( g_pucOutDMaskData != NULL ) {
		free( g_pucOutDMaskData );
		g_pucOutDMaskData = NULL;
	}
	
	if ( g_pucIntelBuffer != NULL ) {
		free( g_pucIntelBuffer );
		g_pucIntelBuffer = NULL;
	}

	if ( g_pLVDSList != NULL ) {
		free( g_pLVDSList );
		g_pLVDSList = NULL;
	}
} 

/***************************************************************
*
* error_handler
*
* Reports the error message.
*
***************************************************************/

void error_handler( short a_siRetCode, char * pszMessage )
{
	const char * pszErrorMessage[] = { "pass",
									   "verification fail",
									   "can't find the file",
									   "wrong file type",
									   "file error",
									   "option error",
									   "crc verification error" };

	strcpy( pszMessage, pszErrorMessage[ -a_siRetCode ] );
}
/***************************************************************
*
* ispVM
*
* The entry point of the ispVM embedded. If the version and CRC
* are verified, then the VME will be processed.
*
***************************************************************/

unsigned char FGETC()
{
    unsigned char ucData;

    if( --g_pVMESize >= 0 )
        ucData = *g_pVMEBuf++;
    else{
        ucData = 0xFF;
        BACKDOOR_MGR_Printf("\r\nEnd of Buffer\r\n");
    }
    return ucData;
}

signed char ispVM( unsigned char *buf, unsigned long bufsize) //const char * a_pszFilename )
{
	char szFileVersion[ 9 ]      = { 0 };
	signed char cRetCode         = 0;
	signed char cIndex           = 0;
	signed char cVersionIndex    = 0;
	unsigned char ucReadByte     = 0;
	
	/***************************************************************
	*
	* Global variables initialization.
	*
	* 09/11/07 NN Added
	***************************************************************/
	g_pucHeapMemory		= NULL;
	g_iHeapCounter		= 0;
	g_iHEAPSize			= 0;
	g_usIntelDataIndex	= 0;
	g_usIntelBufferSize	= 0;
	g_usPreviousSize     = 0;

	/***************************************************************
	*
	* Open a file pointer to the VME file.
	*
	***************************************************************/

//	if ( ( g_pVMEFile = fopen( a_pszFilename, "rb" ) ) == NULL ) {
//		return VME_FILE_READ_FAILURE;
//	}
    g_pVMEBuf = buf;
    g_pVMESize = bufsize;
    
	g_usCalculatedCRC = 0;
	g_usExpectedCRC   = 0;
	ucReadByte = GetByte();    
	
	switch( ucReadByte ) {
	case FILE_CRC:

		/***************************************************************
		*
		* Read and store the expected CRC to do the comparison at the end.  
		* Only versions 3.0 and higher support CRC protection.
		*
		***************************************************************/

		g_usExpectedCRC = FGETC(); //(unsigned char ) fgetc( g_pVMEFile );
		g_usExpectedCRC <<= 8;
		g_usExpectedCRC |= FGETC(); //fgetc( g_pVMEFile );
		

		/***************************************************************
		*
		* Read and store the version of the VME file.
		*
		***************************************************************/

		for ( cIndex = 0; cIndex < 8; cIndex++ ) {
			szFileVersion[ cIndex ] = GetByte();
		}

		break;
	default:

		/***************************************************************
		*
		* Read and store the version of the VME file.  Must be version 2.0.
		*
		***************************************************************/

		szFileVersion[ 0 ] = ( signed char ) ucReadByte;
		for ( cIndex = 1; cIndex < 8; cIndex++ ) {
			szFileVersion[ cIndex ] = GetByte();
		}

		break;
	}

	/***************************************************************
	*
	* Compare the VME file version against the supported version.
	*
	***************************************************************/

	for ( cVersionIndex = 0; g_szSupportedVersions[ cVersionIndex ] != 0; cVersionIndex++ ) {
		for ( cIndex = 0; cIndex < 8; cIndex++ ) {
			if ( szFileVersion[ cIndex ] != g_szSupportedVersions[ cVersionIndex ][ cIndex ] ) {
				cRetCode = VME_VERSION_FAILURE;
				break;
			}	
			cRetCode = 0;
		}

		if ( cRetCode == 0 ) {

			/***************************************************************
			*
			* Found matching version, break.
			*
			***************************************************************/
			BACKDOOR_MGR_Printf("Found matching version\n");
			break;
		}
	}

	if ( cRetCode < 0 ) {

		/***************************************************************
		*
		* VME file version failed to match the supported versions.
		*
		***************************************************************/

	//	fclose( g_pVMEFile );
	//	g_pVMEFile = NULL;
		BACKDOOR_MGR_Printf("not Found matching version\n");
		return VME_VERSION_FAILURE;
	}

	/***************************************************************
	*
	* Enable the JTAG port to communicate with the device.
    * Set the JTAG state machine to the Test-Logic/Reset State.
	*
	***************************************************************/

    ispVMStart();

	/***************************************************************
	*
	* Process the VME file.
	*
	***************************************************************/

    	cRetCode = ispVMCode();

	/***************************************************************
	*
	* Set the JTAG State Machine to Test-Logic/Reset state then disable
    * the communication with the JTAG port.
	*
	***************************************************************/

    	ispVMEnd();
        BACKDOOR_MGR_Printf("vme\n");           
 //   fclose( g_pVMEFile );
//	g_pVMEFile = NULL;


	ispVMFreeMem();

	/***************************************************************
	*
	* Compare the expected CRC versus the calculated CRC.
	*
	***************************************************************/

	if ( cRetCode == 0 && g_usExpectedCRC != 0 && ( g_usExpectedCRC != g_usCalculatedCRC ) ) {
		BACKDOOR_MGR_Printf( "Expected CRC:   0x%.4X\n", g_usExpectedCRC );
		BACKDOOR_MGR_Printf( "Calculated CRC: 0x%.4X\n", g_usCalculatedCRC );
		return VME_CRC_FAILURE;
	}
	
    return ( cRetCode );
}

/***************************************************************
*
* main
*
***************************************************************/

int isp_execute(unsigned char *buf, unsigned long bufsize, int check)
{
//	unsigned short iCommandLineIndex  = 0;
	short siRetCode                   = 0;
//	char szExtension[ 5 ]             = { 0 };
	char szCommandLineArg[ 300 ]      = {'v','m','e' };
//	short sicalibrate                 = 0;

    if ( check )
	{
		siRetCode = calibration();
	}
	else
	{
		vme_out_string( "Processing virtual machine buffer (");
		BACKDOOR_MGR_Printf("%ld", bufsize );
		vme_out_string (")......\n\n");
		siRetCode = ispVM(buf, bufsize);
	}
	if ( siRetCode < 0 ) {
		error_handler( siRetCode, szCommandLineArg );
		vme_out_string( "Failed due to ");
		vme_out_string ( szCommandLineArg );
		vme_out_string ("\n\n");
		vme_out_string( "+=======+\n" );
		vme_out_string( "| FAIL! |\n" );
		vme_out_string( "+=======+\n\n" );
        BACKDOOR_MGR_Printf ("\nExecute Size is %ld\n", g_pVMESize );
	} 
	else {
		vme_out_string( "+=======+\n" );
		vme_out_string( "| PASS! |\n" );
		vme_out_string( "+=======+\n\n" );
	}
	return  siRetCode;
	//exit( siRetCode );
} 

