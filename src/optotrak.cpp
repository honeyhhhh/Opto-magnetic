#include "optotrak.hpp"


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


void certus_init()
{
    // int i;
	// int nCurDevice;
	// int nCurProperty;
	// int nCurFrame;
	// int nCurMarker;
	// int nMarkersToActivate;
	// int nDevices;
	// int nDeviceMarkers;
	// ApplicationDeviceInformation *pdtDevices;
    // DeviceHandle *pdtDeviceHandles;
    // DeviceHandleInfo *pdtDeviceHandlesInfo;

    /*
     * initialization
	 * intialize variables
     */
	// pdtDevices = NULL;
	// pdtDeviceHandles = NULL;
	// pdtDeviceHandlesInfo = NULL;
	// nMarkersToActivate = 0;
	// nDevices = 0;
	// nDeviceMarkers = 0;
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

	if( TransputerLoadSystem( "system" ) != OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();
	}

	sleep(1);

	//fprintf( stdout, "...TransputerInitializeSystem\n" );
    if( TransputerInitializeSystem( OPTO_LOG_ERRORS_FLAG | OPTO_LOG_MESSAGES_FLAG ) != OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();
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

    if( OptotrakSetProcessingFlags( OPTO_LIB_POLL_REAL_DATA |
                                    OPTO_CONVERT_ON_HOST |
                                    OPTO_RIGID_ON_HOST ) )
    {
		return certus_error_exit();
	}
    if( OptotrakLoadCameraParameters( "standard" ) != OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();
	}
    if( OptotrakSetStroberPortTable( dtSettings.nMarkers - 1, 0, 0, 0 ) != OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();
	} /* if */
	if( dtSettings.nMarkers == 0 )
	{
		return certus_error_exit();
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
		return certus_error_exit();
    }

    sleep(1);
    if( OptotrakActivateMarkers( ) != OPTO_NO_ERROR_CODE )
    {
        return certus_error_exit();
    }
	sleep(1);

	return 0;
}


void certus_stop()
{
    if( OptotrakDeActivateMarkers() )
    {
        certus_error_exit();
    } 
	//fprintf( stdout, "...OptotrakStopCollection\n" );
	if( OptotrakStopCollection( ) != OPTO_NO_ERROR_CODE )
	{
		certus_error_exit();
	} 


    TransputerShutdownSystem( );
	// if( pdtDeviceHandlesInfo )
	// {
	// 	for( i = 0; i < nDevices; i++ )
	// 	{
	// 		AllocateMemoryDeviceHandleProperties( &(pdtDeviceHandlesInfo[i].grProperties), 0 );
	// 	} /* for */
	// } /* if */
	// AllocateMemoryDeviceHandles( &pdtDeviceHandles, 0 );
	// AllocateMemoryDeviceHandlesInfo( &pdtDeviceHandlesInfo, pdtDeviceHandles, 0 );
	// free( p3dData );

}



int certus_error_exit()
{
	//fprintf( stdout, "\nAn error has occurred during execution of the program.\n" );
    if( OptotrakGetErrorString( szNDErrorString, MAX_ERROR_STRING_LENGTH + 1 ) == 0 )
    {
        cout << szNDErrorString << endl;
    }
	OptotrakDeActivateMarkers( );
	TransputerShutdownSystem( );

	// if( pdtDeviceHandlesInfo )
	// {
	// 	for( i = 0; i < nDevices; i++ )
	// 	{
	// 		AllocateMemoryDeviceHandleProperties( &(pdtDeviceHandlesInfo[i].grProperties), 0 );
	// 	}
	// }
	// AllocateMemoryDeviceHandles( &pdtDeviceHandles, 0 );
	// AllocateMemoryDeviceHandlesInfo( &pdtDeviceHandlesInfo, pdtDeviceHandles, 0 );
	// free( p3dData );

    return 1;

} 



void c_get_frame()
{

    /*
     * Get frame of 3D data.
     */
    unsigned int uFlags;
    unsigned int uElements;
    unsigned int uFrameNumber;
	unsigned int preFrameNumber = -1;

	int ndata = 0;
	
	Position3d *p3dData = NULL;;

	p3dData = (Position3d* )malloc( (dtSettings.nMarkers - 1) * sizeof(Position3d) );

    std::ofstream fs("../opt_timestamp.txt", std::ios::out);
    std::ofstream fd("../opt_data.txt", std::ios::out);
    std::ofstream fn("../opt_num.txt", std::ios::out);

    std::vector<uint64_t> opt_Time(2500);
    std::vector<std::vector<double> > opt_Data (2500, std::vector<double>(3));
    std::vector<unsigned int> frame_Number(2500);


    // 进入屏障
    ::EnterSynchronizationBarrier(sb, SYNCHRONIZATION_BARRIER_FLAGS_SPIN_ONLY);

    auto t1 = std::chrono::steady_clock::now();


    while (1)
    {
		
		if (WaitForSingleObject(gDoneEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}

        if(DataGetLatest3D(&uFrameNumber, &uElements, &uFlags, p3dData))
        {
			;
        }
		if (preFrameNumber == uFrameNumber)
		{
			continue;
		}


        auto t = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
        for(int nCurMarker = 0; nCurMarker < dtSettings.nMarkers - 1; nCurMarker++)
        {
			opt_Time.push_back(t);
			opt_Data.push_back(std::vector<double>(std::begin(p3dData), std::end(p3dData)));
			frame_Number.push_back(uFrameNumber);

			preFrameNumber = uFrameNumber;
			ndata++;
        }


    }

    auto t2 = std::chrono::steady_clock::now();
    double dr_ms = std::chrono::duration<double,std::milli>(t2-t1).count();

	// save
	std::cout << "size" << opt_Time.size() << "\t" << opt_Data.size() << std::endl;
	for (auto t : opt_Time)
		fs << t << "\n";
	for (auto t : opt_Data)
		fd << t[0] << " " << t[1] << " " << t[2] << "\n";
	for (auto t : frame_Number)
		fn << t << "\n";
    auto t3 = std::chrono::steady_clock::now();

    std::cout << "opt_frame_cout " << ndata << "\t" << "times :" << t1.time_since_epoch().count() / 10000 << "~" << t2.time_since_epoch().count() / 10000<< "~" << t3.time_since_epoch().count() / 10000 << std::endl;


    fs.close();
    fn.close();
    fd.close();
	free(p3dData);
    _endthreadex(0);


}
