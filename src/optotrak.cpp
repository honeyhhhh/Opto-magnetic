#include "optotrak.hpp"



OptotrakSettings dtSettings;
char szNDErrorString[MAX_ERROR_STRING_LENGTH + 1];
char szProperty[32];

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


int certus_init()
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
        std::cout << szNDErrorString << std::endl;
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



unsigned __stdcall c_get_frame(LPVOID)
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

    std::ofstream fs("../dataset/opt_timestamp.txt", std::ios::out);
    std::ofstream fd("../dataset/opt_data.txt", std::ios::out);
    std::ofstream fn("../dataset/opt_num.txt", std::ios::out);

    std::vector<uint64_t> opt_Time(2500);
    std::vector<std::vector<double> > opt_Data (2500, std::vector<double>(3));
    std::vector<unsigned int> frame_Number(2500);

	// 进入屏障
	std::cout << "certus thread into barrier" << std::endl;
    PVOID p = &sb;
    auto barrier = (PSYNCHRONIZATION_BARRIER)p;
    ::EnterSynchronizationBarrier(barrier, SYNCHRONIZATION_BARRIER_FLAGS_SPIN_ONLY);
	std::cout << "certus thread start" << std::endl;

    auto t1 = std::chrono::system_clock::now();


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
			fs << t << "\n";
			fd << p3dData[nCurMarker].x << " " << p3dData[nCurMarker].y << " " << p3dData[nCurMarker].z << "\n";
			fn << uFrameNumber << "\n";
			// opt_Time.push_back(t);
			// opt_Data.push_back(std::vector<double>({p3dData[nCurMarker].x, p3dData[nCurMarker].y, p3dData[nCurMarker].z}));
			// frame_Number.push_back(uFrameNumber);

			preFrameNumber = uFrameNumber;
			ndata++;
        }


    }

    auto t2 = std::chrono::system_clock::now();


    std::cout << "opt_frame_cout " << ndata << "\t";
    std::cout << "times :" << t1.time_since_epoch().count() << "~";
    std::cout << t2.time_since_epoch().count() << std::endl;

    fs.close();
    fn.close();
    fd.close();
	free(p3dData);
    _endthreadex(0);
	return 0;


}


void c_get_frame2()
{
    /*
     * Get frame of 3D data.
     */
    unsigned int uFlags;
    unsigned int uElements;
    unsigned int uFrameNumber;
	
	Position3d *p3dData = NULL;;

	p3dData = (Position3d* )malloc( (dtSettings.nMarkers - 1) * sizeof(Position3d) );

    std::ofstream fs("../static_data/opt_timestamp.txt", std::ios::out | std::ios::app);
    std::ofstream fd("../static_data/opt_data.txt", std::ios::out | std::ios::app);
    std::ofstream fn("../static_data/opt_num.txt", std::ios::out | std::ios::app);

    if(DataGetLatest3D(&uFrameNumber, &uElements, &uFlags, p3dData))
    {
		;
    }
    auto t = std::chrono::system_clock::now().time_since_epoch().count() / 10000;

    for(int nCurMarker = 0; nCurMarker < dtSettings.nMarkers - 1; nCurMarker++)
    {
		fs << t << "\n";
		fd << p3dData[nCurMarker].x << " " << p3dData[nCurMarker].y << " " << p3dData[nCurMarker].z << "\n";
		fn << uFrameNumber << "\n";
    }

    std::cout << "opt_frame" << "\n";
    fs.close();
    fn.close();
    fd.close();
	free(p3dData);

}


void c_get_frame4()
{
    /*
     * Get frame of 3D data.
     */
    unsigned int uFlags;
    unsigned int uElements;
    unsigned int uFrameNumber;
	
	Position3d *p3dData = NULL;
	struct OptotrakRigidStruct *p6dData = NULL;
	int nTotalRigids = 1;

	p3dData = (Position3d* )malloc( (dtSettings.nMarkers) * sizeof(Position3d) );
	p6dData = (struct OptotrakRigidStruct*)malloc( nTotalRigids * sizeof( struct OptotrakRigidStruct ) );

    std::ofstream fs("../static_data/opt_timestamp4.txt", std::ios::out | std::ios::app);
    std::ofstream fd("../static_data/opt_data4.txt", std::ios::out | std::ios::app);
    std::ofstream fn("../static_data/opt_num4.txt", std::ios::out | std::ios::app);

    if( DataGetLatestTransforms2( &uFrameNumber, &uElements, &uFlags, p6dData, p3dData ) )
    {
        ;
    }
    auto t = std::chrono::system_clock::now().time_since_epoch().count() / 10000;




    for(int nCurMarker = 0; nCurMarker < dtSettings.nMarkers; nCurMarker++)
    {
		fs << t << "\n";
		fd << nCurMarker << " " << p3dData[nCurMarker].x << " " << p3dData[nCurMarker].y << " " << p3dData[nCurMarker].z << "\n";
		fn << uFrameNumber << "\n";
    }
    for( int nCurRigid = 0; nCurRigid < nTotalRigids; ++nCurRigid )
    {
		if( p6dData[nCurRigid].flags & OPTOTRAK_UNDETERMINED_FLAG )
		{
			fd << "Missing" << "\n";
		}
		else
		{
			fd << p6dData[nCurRigid].transformation.euler.translation.x << " " << p6dData[nCurRigid].transformation.euler.translation.y << " " << p6dData[nCurRigid].transformation.euler.translation.z << "\n";
			// fd << "YPR " << p6dData[nCurRigid].transformation.euler.rotation.yaw << " " << p6dData[nCurRigid].transformation.euler.rotation.pitch << " " << p6dData[nCurRigid].transformation.euler.rotation.roll << "\n";
			fd << p6dData[nCurRigid].transformation.quaternion.rotation.q0 << " " << p6dData[nCurRigid].transformation.quaternion.rotation.qx <<
			" " << p6dData[nCurRigid].transformation.quaternion.rotation.qy << " " << p6dData[nCurRigid].transformation.quaternion.rotation.qz << " " << p6dData[nCurRigid].QuaternionError  <<"\n";
		}
    } 




    std::cout << "opt_frame" << "\n";
    fs.close();
    fn.close();
    fd.close();
	free(p3dData);
	free(p6dData);
}


int certus_init4()
{
	DeviceHandleProperty dtProperty;
	dtSettings.nMarkers = 4;
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

    if( TransputerInitializeSystem( OPTO_LOG_ERRORS_FLAG | OPTO_LOG_MESSAGES_FLAG ) != OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();
	}

	sleep(5);

	int n;
	OptotrakGetNumberDeviceHandles(&n);

    if( OptotrakDeviceHandleEnable(1) != OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();	
	}
	sleep(1);

    if( OptotrakDeviceHandleEnable(2) != OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();	
	}
	sleep(1);


	dtProperty.uPropertyID = 2;
	dtProperty.dtPropertyType = DH_PROPERTY_TYPE_INT;
	dtProperty.dtData.nData = 0;
	if( OptotrakDeviceHandleSetProperties( 1, &dtProperty, 1 )!= OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();
	}

	sleep(1);

	if( RigidBodyAddFromDeviceHandle( 2, 0, 0 ) != OPTO_NO_ERROR_CODE )
	{
		return certus_error_exit();
	} 


	sleep(1);

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

	if( dtSettings.nMarkers == 0 )
	{
		return certus_error_exit();
	}

    if( OptotrakSetupCollection( dtSettings.nMarkers,
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









    if( RigidBodyChangeSettings(
            0,   			/* ID associated with this rigid body. */
            4,              /* Minimum number of markers which must be seen
                               before performing rigid body calculations.*/
            60,             /* Cut off angle for marker inclusion in calcs.*/
            (float)0.25,    /* Maximum 3-D marker error for this rigid body. */
            (float)1.0,     /* Maximum raw sensor error for this rigid body. */
            (float)1.0,     /* Maximum 3-D RMS marker error for this rigid body. */
            (float)1.0,     /* Maximum raw sensor RMS error for this rigid body. */
            OPTOTRAK_QUATERN_RIGID_FLAG | OPTOTRAK_RETURN_QUATERN_FLAG ) )
    {
        return certus_error_exit();
    }









	return 0;
}
