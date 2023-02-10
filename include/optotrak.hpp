#ifndef _OPTOTRAK_H 
#define _OPTOTRAK_H 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <fstream>
extern "C"
{

#include "ndtypes.h"
#include "ndpack.h"
#include "ndopto.h"

#include "certus_aux.h"
// #include "ot_aux.h"
}


#define SAMPLE_MARKERFREQ	2500.0f
#define SAMPLE_FRAMEFREQ	200.0f
#define SAMPLE_DUTYCYCLE	0.35f
#define SAMPLE_VOLTAGE		7.0f
#define SAMPLE_STREAMDATA	0

#define CERTUS_SAMPLE_STROBER_MARKERSTOFIRE		2
#define CERTUS_SAMPLE_3020_STROBER_MARKERSTOFIRE	6


static void sleep( unsigned int uSec )
{
	Sleep(uSec * 1000 );
}




static void DisplayFloat( float fFloat )
{
	if( fFloat < MAX_NEGATIVE )
	{
		//fprintf( stdout, "%10s%5s", "MISSING", "" );
	}
	else
	{
		//fprintf( stdout, "%10.2f%5s", fFloat, "" );
	} /* if */
} /* DisplayFloat */


static void DisplayPosition3d( Position3d dtPosition3d	)
{
	//fprintf( stdout, "X" );
	DisplayFloat( dtPosition3d.x );
	//fprintf( stdout, "Y " );
	DisplayFloat( dtPosition3d.y );
	//fprintf( stdout, "Z " );
	DisplayFloat( dtPosition3d.z );

} /* DisplayPosition3d */


static void DisplayMarker( int nMarker, Position3d dtPosition3d )
{
	//fprintf( stdout, "Marker_%.3d: ", nMarker );
	DisplayPosition3d( dtPosition3d );
	//fprintf( stdout, "\n" );

} /* DisplayMarker */

int certus_test()
{
    // logg = fopen("../log_certus.txt", "w");
    // if (logg == NULL)
    // {
    //     std::cout << "failed create log" << endl;
    // }
    // //fprintf(logg, "begin certus test\n");
	OptotrakSettings dtSettings;
    char szNDErrorString[MAX_ERROR_STRING_LENGTH + 1];
	char szProperty[32];
    int i;
	int nCurDevice;
	int nCurProperty;
	int nCurFrame;
	int nCurMarker;
	int nMarkersToActivate;
	int nDevices;
	int nDeviceMarkers;
	ApplicationDeviceInformation *pdtDevices;
    DeviceHandle *pdtDeviceHandles;
    DeviceHandleInfo *pdtDeviceHandlesInfo;
    unsigned int uFlags;
    unsigned int uElements;
    unsigned int uFrameNumber;
	Position3d *p3dData;
	char *pChar;
	char szInput[10];

    /*
     * initialization
	 * intialize variables
     */
	pdtDevices = NULL;
	pdtDeviceHandles = NULL;
	pdtDeviceHandlesInfo = NULL;
	p3dData = NULL;
	nMarkersToActivate = 0;
	nDevices = 0;
	nDeviceMarkers = 0;
	dtSettings.nMarkers = 2;
	dtSettings.fFrameFrequency = SAMPLE_FRAMEFREQ;
	dtSettings.fMarkerFrequency = SAMPLE_MARKERFREQ;
	dtSettings.nThreshold = 30;
	dtSettings.nMinimumGain = 160;
	dtSettings.nStreamData = SAMPLE_STREAMDATA;
	dtSettings.fDutyCycle = SAMPLE_DUTYCYCLE;
	dtSettings.fVoltage = SAMPLE_VOLTAGE;
	dtSettings.fCollectionTime = 1.0;
	dtSettings.fPreTriggerTime = 0.0;

	//fprintf( stdout, "...TransputerLoadSystem\n" );
	if( TransputerLoadSystem( "system" ) != OPTO_NO_ERROR_CODE )
	{
		//goto ERROR_EXIT;
	}

	sleep( 1 );

	//fprintf( stdout, "...TransputerInitializeSystem\n" );
    if( TransputerInitializeSystem( OPTO_LOG_ERRORS_FLAG | OPTO_LOG_MESSAGES_FLAG ) != OPTO_NO_ERROR_CODE )
	{
		//goto ERROR_EXIT;
	}
	// //fprintf( stdout, "...DetermineSystem\n" );
	// if( uDetermineSystem( ) != OPTOTRAK_CERTUS_FLAG )
	// {
	// 	//goto PROGRAM_COMPLETE;
	// }
	// //fprintf( stdout, "...DetermineStroberConfiguration\n" );
	// if( DetermineStroberConfiguration( &pdtDeviceHandles, &pdtDeviceHandlesInfo, &nDevices ) != OPTO_NO_ERROR_CODE )
	// {
	// 	//goto ERROR_EXIT;
	// }
    // ApplicationStoreDeviceProperties( &pdtDevices, pdtDeviceHandlesInfo, nDevices );
	// for( nCurDevice = 0; nCurDevice < nDevices; nCurDevice++ )
	// {
	// 	nMarkersToActivate = pdtDevices[nCurDevice].b3020Capability?
	// 						   CERTUS_SAMPLE_3020_STROBER_MARKERSTOFIRE : CERTUS_SAMPLE_STROBER_MARKERSTOFIRE;

	// 	SetMarkersToActivateForDevice( &(pdtDevices[nCurDevice]), pdtDeviceHandlesInfo[nCurDevice].pdtHandle->nID, nMarkersToActivate );
	// }
	// //fprintf( stdout, "\n" );
	// for( nCurDevice = 0; nCurDevice < nDevices; nCurDevice++ )
	// {
	// 	if( GetDevicePropertiesFromSystem( &(pdtDeviceHandlesInfo[nCurDevice]) ) != OPTO_NO_ERROR_CODE )
	// 	{
	// 		//goto ERROR_EXIT;
	// 	}
	// }

	// if( ApplicationStoreDeviceProperties( &pdtDevices, pdtDeviceHandlesInfo, nDevices ) != OPTO_NO_ERROR_CODE )
	// {
	// 	//goto ERROR_EXIT;
	// }
	// if( nDevices == 0 )
	// {
	// 	//fprintf( stdout, ".........no devices detected.\n" );
	// 	//goto PROGRAM_COMPLETE;
	// }
	// ApplicationDetermineCollectionParameters( nDevices, pdtDevices, &dtSettings );

	//fprintf( stdout, "...OptotrakSetProcessingFlags\n" );
    if( OptotrakSetProcessingFlags( OPTO_LIB_POLL_REAL_DATA |
                                    OPTO_CONVERT_ON_HOST |
                                    OPTO_RIGID_ON_HOST ) )
    {
        //goto ERROR_EXIT;
    }
	//fprintf( stdout, "...OptotrakLoadCameraParameters\n" );
    if( OptotrakLoadCameraParameters( "standard" ) != OPTO_NO_ERROR_CODE )
	{
		//goto ERROR_EXIT;
	}
    //fprintf( stdout, "...OptotrakSetStroberPortTable\n" );
    if( OptotrakSetStroberPortTable( dtSettings.nMarkers - 1, 0, 0, 0 ) != OPTO_NO_ERROR_CODE )
	{
		//goto ERROR_EXIT;
	} /* if */
	if( dtSettings.nMarkers == 0 )
	{
		//fprintf( stdout, "Error: There are no markers to be activated.\n" );
		//goto ERROR_EXIT;
	}
	//fprintf( stdout, "...OptotrakSetupCollection\n" );
	//fprintf( stdout, ".....%d, %.2f, %.0f, %d, %d, %d, %.2f, %.2f, %.0f, %.0f\n",
								//  dtSettings.nMarkers - 1,
			                    //  dtSettings.fFrameFrequency,
				                //  dtSettings.fMarkerFrequency,
					            //  dtSettings.nThreshold,
						        //  dtSettings.nMinimumGain,
							    //  dtSettings.nStreamData,
								//  dtSettings.fDutyCycle,
								//  dtSettings.fVoltage,
								//  dtSettings.fCollectionTime,
								//  dtSettings.fPreTriggerTime );
    if( OptotrakSetupCollection( dtSettings.nMarkers - 1,
			                     dtSettings.fFrameFrequency,
				                 dtSettings.fMarkerFrequency,
					             dtSettings.nThreshold,
						         dtSettings.nMinimumGain,
							     dtSettings.nStreamData,
								 dtSettings.fDutyCycle,
								 dtSettings.fVoltage,
								 dtSettings.fCollectionTime,
								 dtSettings.fPreTriggerTime,
								 OPTOTRAK_NO_FIRE_MARKERS_FLAG | OPTOTRAK_BUFFER_RAW_FLAG | OPTOTRAK_SWITCH_AND_CONFIG_FLAG ) != OPTO_NO_ERROR_CODE )
    {
        //goto ERROR_EXIT;
    }

    sleep( 1 );

	//fprintf( stdout, "...OptotrakActivateMarkers\n" );
    if( OptotrakActivateMarkers( ) != OPTO_NO_ERROR_CODE )
    {
        //goto ERROR_EXIT;
    }
	sleep( 1);


    /*
     * Get frame of 3D data.
     */

	p3dData = (Position3d*)malloc( dtSettings.nMarkers * sizeof( Position3d ) );

    std::ofstream fs("../opt_timestamp.txt", std::ios::out);
    std::ofstream fd("../opt_data.txt", std::ios::out);



    int ndata = 0;
    cout << "optotrak ready !" << endl;
    while (!cam_ready){ std::this_thread::sleep_for(std::chrono::microseconds(50));}
    cout << "optotrak start !" << endl;

    while (1)
    {
        //fprintf( stdout, "\n" );
        if( DataGetLatest3D( &uFrameNumber, &uElements, &uFlags, p3dData ) )
        {
            ;
        }
        auto t = std::chrono::system_clock::now().time_since_epoch().count() / 10000;

        //fprintf( stdout, "Frame Number: %8u\n", uFrameNumber );
        //fprintf( stdout, "Elements    : %8u\n", uElements );
        //fprintf( stdout, "Flags       : 0x%04x\n", uFlags );
        for( nCurMarker = 0; nCurMarker < dtSettings.nMarkers; nCurMarker++ )
        {
            fs << t << "\n";
			fd << p3dData[nCurMarker].x << " " << p3dData[nCurMarker].y << " " << p3dData[nCurMarker].z << "\n";
        } 


        ndata++;
        if (ndata > 200)
            break;

    }







	/*
	 * Stop the collection.
	 */

    if( OptotrakDeActivateMarkers() )
    {
        ////goto ERROR_EXIT;
    } 
	//fprintf( stdout, "...OptotrakStopCollection\n" );
	if( OptotrakStopCollection( ) != OPTO_NO_ERROR_CODE )
	{
		//goto ERROR_EXIT;
	} 



PROGRAM_COMPLETE:
    /*
     * CLEANUP
     */
	//fprintf( stdout, "\n" );
	//fprintf( stdout, "...TransputerShutdownSystem\n" );
    TransputerShutdownSystem( );

	/*
	 * free all memory
	 */
	if( pdtDeviceHandlesInfo )
	{
		for( i = 0; i < nDevices; i++ )
		{
			AllocateMemoryDeviceHandleProperties( &(pdtDeviceHandlesInfo[i].grProperties), 0 );
		} /* for */
	} /* if */
	AllocateMemoryDeviceHandles( &pdtDeviceHandles, 0 );
	AllocateMemoryDeviceHandlesInfo( &pdtDeviceHandlesInfo, pdtDeviceHandles, 0 );
	free( p3dData );

	return 0;

ERROR_EXIT:
	//fprintf( stdout, "\nAn error has occurred during execution of the program.\n" );
    if( OptotrakGetErrorString( szNDErrorString, MAX_ERROR_STRING_LENGTH + 1 ) == 0 )
    {
        //fprintf( stdout, szNDErrorString );
    }

	//fprintf( stdout, "\n\n...TransputerShutdownSystem\n" );
	OptotrakDeActivateMarkers( );
	TransputerShutdownSystem( );

	if( pdtDeviceHandlesInfo )
	{
		for( i = 0; i < nDevices; i++ )
		{
			AllocateMemoryDeviceHandleProperties( &(pdtDeviceHandlesInfo[i].grProperties), 0 );
		}
	}
	AllocateMemoryDeviceHandles( &pdtDeviceHandles, 0 );
	AllocateMemoryDeviceHandlesInfo( &pdtDeviceHandlesInfo, pdtDeviceHandles, 0 );
	free( p3dData );

    return 1;

} 




















#endif	// _OPTOTRAK_H