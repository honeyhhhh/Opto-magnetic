#include "test.hpp"

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <chrono>
#include <iostream>
FILE *logg = NULL;

static void sleep( unsigned int uSec )
{
	Sleep(uSec * 1000 );
}



// 非同步 采集图像
int Grab_MultipleCameras()
{
    int exitCode = 0;

    PylonInitialize();

    try
    {
        // Get the transport layer factory.
        CTlFactory& tlFactory = CTlFactory::GetInstance();

        // Get all attached devices and exit application if no device is found.
        DeviceInfoList_t devices;
        if (tlFactory.EnumerateDevices( devices ) == 0)
        {
            throw RUNTIME_EXCEPTION( "No camera present." );
        }

        // Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
        CInstantCameraArray cameras( min( devices.size(), c_maxCamerasToUse ) );

        // Create and attach all Pylon Devices.
        for (size_t i = 0; i < cameras.GetSize(); ++i)
        {
            cameras[i].Attach( tlFactory.CreateDevice( devices[i] ) );
            cameras[i].Open(); //打开才能读参数
            

            // Print the model name of the camera.
            cout << "Using device " << 
            cameras[i].GetDeviceInfo().GetModelName() << " " <<
            cameras[i].GetDeviceInfo().GetIpAddress() << " " <<
            cameras[i].GetDeviceInfo().GetDeviceClass() <<  " " <<
            cameras[i].GetDeviceInfo().GetSerialNumber() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("OffsetX")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("OffsetY")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("ExposureTimeAbs")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("ExposureMode")).ToString() << " " <<
            // CParameter(cameras[i].GetNodeMap().GetNode("ExposureStartDelayAbs")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("AcquisitionMode")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("AcquisitionFrameRateEnable")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("AcquisitionFrameRateAbs")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("Width")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("Height")).ToString() << " " <<
            CEnumParameter(cameras[i].GetNodeMap().GetNode("PixelFormat")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("TriggerSource")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("TriggerActivation")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("TriggerSelector")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("TriggerMode")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("ReadoutTimeAbs")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("ResultingFrameRateAbs")).ToString() << " " <<
            CParameter(cameras[i].GetNodeMap().GetNode("ShutterMode")).ToString() << " " <<

            endl;
        }

        // Starts grabbing for all cameras starting with index 0. The grabbing
        // is started for one camera after the other. That's why the images of all
        // cameras are not taken at the same time.
        // However, a hardware trigger setup can be used to cause all cameras to grab images synchronously.
        // According to their default configuration, the cameras are
        // set up for free-running continuous acquisition.
        cameras.StartGrabbing();

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        // Grab c_countOfImagesToGrab from the cameras.
        for (uint32_t i = 0; i < c_countOfImagesToGrab && cameras.IsGrabbing(); ++i)
        {
            cameras.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException );

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
                // When the cameras in the array are created the camera context value
                // is set to the index of the camera in the array.
                // The camera context is a user settable value.
                // This value is attached to each grab result and can be used
                // to determine the camera that produced the grab result.
                intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();

#ifdef PYLON_WIN_BUILD
                // Show the image acquired by each camera in the window related to each camera.
                Pylon::DisplayImage( cameraContextValue, ptrGrabResult );
               
#endif

                // Print the index and the model name of the camera.
                cout << "Camera " << cameraContextValue << ": " << cameras[cameraContextValue].GetDeviceInfo().GetModelName() << endl;

                // Now, the image data can be processed.
                cout << "GrabSucceeded: " << ptrGrabResult->GrabSucceeded() << endl;
                cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
                cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
                const uint8_t* pImageBuffer = (uint8_t*) ptrGrabResult->GetBuffer();
                cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;
            }
            else
            {
                cout << "Error: " << std::hex << ptrGrabResult->GetErrorCode() << std::dec << " " << ptrGrabResult->GetErrorDescription() << endl;
            }
        }


        cameras.StopGrabbing();
        for (size_t i = 0; i < cameras.GetSize(); ++i)
        {
            cameras[i].Close();
        }
            
    }
    catch (const GenericException& e)
    {
        // Error handling
        cerr << "An exception occurred." << endl
            << e.GetDescription() << endl;
        exitCode = 1;
    }

    // Comment the following two lines to disable waiting on exit.
    cerr << endl << "Press enter to exit." << endl;
    while (cin.get() != '\n');

    


    // Releases all pylon resources.
    PylonTerminate();

    return exitCode;
}




void GigEcameraCreateWithIp()
{
    CTlFactory& TlFactory = CTlFactory::GetInstance();
    CDeviceInfo di;
    di.SetIpAddress( "192.168.0.101");
    CInstantCamera camera( TlFactory.CreateDevice( di ) );
}




void get_frame(ndicapi* device)
{
    auto t1 = std::chrono::steady_clock::now();
    int frame_count = 0;
    while(1)
    {
        std::chrono::system_clock::time_point tt = std::chrono::system_clock::now();
        std::chrono::nanoseconds d = tt.time_since_epoch();

        const char* re = ndiTX(device, 0x0801);

        ulong frame_number =  ndiGetTXFrame(device, 10);
        double trans[8];
        int tfStatus = ndiGetTXTransform(device, 10, trans);
        frame_count++;
        if (tfStatus == NDI_MISSING || tfStatus == NDI_DISABLED)
        {
            fprintf(logg, "bad frame\n");
        }

        fprintf(logg, "frame %lu time:%llu\n%.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf\n", frame_number, d.count(), 
                                                trans[0], trans[1], trans[2], trans[3], trans[4], trans[5], trans[6], trans[7]);

        if (frame_number > 400)
        {
            break;
        }
    }

    auto t2 = std::chrono::steady_clock::now();
    double dr_ms = std::chrono::duration<double,std::milli>(t2-t1).count();

    fprintf(logg, "frame_count :%d ; use time :%lf\n", frame_count, dr_ms);



}




int aurora_test()
{

    // std::cout is not use
    logg = fopen("../log.txt", "w");
    if (logg == NULL)
    {
        std::cout << "failed create log" << endl;
    }
    fprintf(logg, "begin aurora test\n");

    bool checkDSR = false;
    ndicapi* device(nullptr);
    const char* name("COM8");


    const int MAX_SERIAL_PORTS = 0;
    /*
        Probes the system's serial ports to
        find the name of the serial port and check we can connect
        :return port_name: the name of the port
        :raises: IOError if port not found or port probe fails
    */
    for (int i = 0; i < MAX_SERIAL_PORTS; ++i)
    {
        name = ndiSerialDeviceName(i);
        int result = ndiSerialProbe(name,checkDSR);
        if (result == NDI_OKAY)
        {
            fprintf(logg, "found serial :%s\n", name);
            break;
        }
    }
    if (name != nullptr)
    {
        int result = ndiSerialProbe(name,checkDSR);
        // 打开串口
        fprintf(logg, "open serial :%s\n", name);
        device = ndiOpenSerial(name);
    }
    else
    {
        std::cout << "not found !" << std::endl;
    }

    if (device != nullptr)
    {
        const char* reply = ndiCo2mmand(device, "VER:4");
        fprintf(logg, "------ system version ------ \n %s", reply);


        /*
            Init system and  Set System COM Parms
        */
        fprintf(logg, "----- Init system and  Set System COM Parms -----\n");
        reply = ndiCommand(device, "INIT:");
        fprintf(logg, "send INIT command :%s\n", reply);

        if (strncmp(reply, "ERROR", strlen(reply)) == 0 || ndiGetError(device) != NDI_OKAY)
        {
            std::cerr << "Error when sending command: " << ndiErrorString(ndiGetError(device)) << std::endl;
            return EXIT_FAILURE;
        }

        reply = ndiCommand(device, "COMM:%d%03d%d", NDI_115200, NDI_8N1, NDI_NOHANDSHAKE);
        fprintf(logg, "send connect serial command :%s\n", reply);


        /*
            Initialize All Port handle
        */
        fprintf(logg, "----- Initialize All Port handle -----\n");
        reply = ndiCommand(device, "PHSR:01");

        fprintf(logg, "send PHSR:01 command Are there port handles to be freed?:%s\n", reply);
        reply = ndiCommand(device, "PHSR:02");
        fprintf(logg, "send PHSR:02 command Are there port handles to be initialized?:%s\n", reply);
        int number_of_tools = ndiGetPHSRNumberOfHandles(device); // 电磁标记点
        fprintf(logg, "found %d PHSRNumberOfHandles need init\n", number_of_tools);

        while (number_of_tools > 0)
        {
            for (int ndi_tool_index = 0; ndi_tool_index < number_of_tools; ndi_tool_index++)
            {
                int port_handle = ndiGetPHSRHandle(device, ndi_tool_index);
                fprintf(logg, "The PHINF command can be used to get detailed information about the port handle: %02X\n",port_handle);
                reply = ndiCommand(device, "PINIT:%02X",port_handle);
                fprintf(logg, "send PINIT:%02X command Initialize port handles:%s\n", port_handle,reply);

            }
            reply = ndiCommand(device, "PHSR:02");
            fprintf(logg, "send PHSR:02 command Are there port handles to be initialized?:%s\n", reply);
            number_of_tools = ndiGetPHSRNumberOfHandles(device);
            fprintf(logg, "found %d PHSRNumberOfHandles\n", number_of_tools);
        }

        /*
            Enable All Ports
        */
        fprintf(logg, "----- Enable All Ports -----\n");

        reply = ndiCommand(device, "PHSR:03");
        fprintf(logg, "send PHSR:03 command  Are there port handles to be enabled?:%s\n", reply);
        number_of_tools = ndiGetPHSRNumberOfHandles(device); // 电磁标记点
        fprintf(logg, "found %d PHSRNumberOfHandles need activate\n", number_of_tools);

        for (int ndi_tool_index = 0; ndi_tool_index < number_of_tools; ndi_tool_index++)
        {
            int port_handle = ndiGetPHSRHandle(device, ndi_tool_index);
            fprintf(logg, "The PHINF command can be used to get detailed information about the port handle: %02X\n",port_handle);
            reply = ndiCommand(device, "PENA:%02X%c",port_handle,'D');
            fprintf(logg, "send PENA:%02X%c command Enable port handles:%s\n", port_handle,'D',reply);


        }
        

        /*
            Data tansform mode
            "TX Mode (ASCII return for allocated port handles)", TX:0801
            "BX Mode (Binary return for allocated port handles)", fast but aurora 008 not support
        */
        


        /*
            Start tracking
        */

        reply = ndiCommand(device, "TSTART:");
        fprintf(logg, "----- Start tracking -----\n");
        if (strncmp(reply, "ERROR", strlen(reply)) == 0 || ndiGetError(device) != NDI_OKAY)
        {
            std::cerr << "Error when sending command: " << ndiErrorString(ndiGetError(device)) << std::endl;
            return EXIT_FAILURE;
        }


        /*
            get frame

        */
        get_frame(device);


        /*
            Stop tracking
        */
        reply = ndiCommand(device, "TSTOP:");
        fprintf(logg, "----- Stop tracking -----\n");
        if (strncmp(reply, "ERROR", strlen(reply)) == 0 || ndiGetError(device) != NDI_OKAY)
        {
            std::cerr << "Error when sending command: " << ndiErrorString(ndiGetError(device)) << std::endl;
            return EXIT_FAILURE;
        }

        // 关闭串口
        ndiCloseSerial(device);
    }
    else
    {
        std::cout << "not device !" << std::endl;
    }

    fprintf(logg, "end\n");

    fclose(logg);
    return 0;


}



static void DisplayFloat( float fFloat )
{
	if( fFloat < MAX_NEGATIVE )
	{
		fprintf( stdout, "%10s%5s", "MISSING", "" );
	}
	else
	{
		fprintf( stdout, "%10.2f%5s", fFloat, "" );
	} /* if */
} /* DisplayFloat */


static void DisplayPosition3d( Position3d dtPosition3d	)
{
	fprintf( stdout, "X" );
	DisplayFloat( dtPosition3d.x );
	fprintf( stdout, "Y " );
	DisplayFloat( dtPosition3d.y );
	fprintf( stdout, "Z " );
	DisplayFloat( dtPosition3d.z );

} /* DisplayPosition3d */


static void DisplayMarker( int nMarker, Position3d dtPosition3d )
{
	fprintf( stdout, "Marker_%.3d: ", nMarker );
	DisplayPosition3d( dtPosition3d );
	fprintf( stdout, "\n" );

} /* DisplayMarker */

int certus_test()
{
    // logg = fopen("../log_certus.txt", "w");
    // if (logg == NULL)
    // {
    //     std::cout << "failed create log" << endl;
    // }
    // fprintf(logg, "begin certus test\n");
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

	fprintf( stdout, "...TransputerLoadSystem\n" );
	if( TransputerLoadSystem( "system" ) != OPTO_NO_ERROR_CODE )
	{
		goto ERROR_EXIT;
	}

	sleep( 1 );

	fprintf( stdout, "...TransputerInitializeSystem\n" );
    if( TransputerInitializeSystem( OPTO_LOG_ERRORS_FLAG | OPTO_LOG_MESSAGES_FLAG ) != OPTO_NO_ERROR_CODE )
	{
		goto ERROR_EXIT;
	}
	// fprintf( stdout, "...DetermineSystem\n" );
	// if( uDetermineSystem( ) != OPTOTRAK_CERTUS_FLAG )
	// {
	// 	goto PROGRAM_COMPLETE;
	// }
	// fprintf( stdout, "...DetermineStroberConfiguration\n" );
	// if( DetermineStroberConfiguration( &pdtDeviceHandles, &pdtDeviceHandlesInfo, &nDevices ) != OPTO_NO_ERROR_CODE )
	// {
	// 	goto ERROR_EXIT;
	// }
    // ApplicationStoreDeviceProperties( &pdtDevices, pdtDeviceHandlesInfo, nDevices );
	// for( nCurDevice = 0; nCurDevice < nDevices; nCurDevice++ )
	// {
	// 	nMarkersToActivate = pdtDevices[nCurDevice].b3020Capability?
	// 						   CERTUS_SAMPLE_3020_STROBER_MARKERSTOFIRE : CERTUS_SAMPLE_STROBER_MARKERSTOFIRE;

	// 	SetMarkersToActivateForDevice( &(pdtDevices[nCurDevice]), pdtDeviceHandlesInfo[nCurDevice].pdtHandle->nID, nMarkersToActivate );
	// }
	// fprintf( stdout, "\n" );
	// for( nCurDevice = 0; nCurDevice < nDevices; nCurDevice++ )
	// {
	// 	if( GetDevicePropertiesFromSystem( &(pdtDeviceHandlesInfo[nCurDevice]) ) != OPTO_NO_ERROR_CODE )
	// 	{
	// 		goto ERROR_EXIT;
	// 	}
	// }

	// if( ApplicationStoreDeviceProperties( &pdtDevices, pdtDeviceHandlesInfo, nDevices ) != OPTO_NO_ERROR_CODE )
	// {
	// 	goto ERROR_EXIT;
	// }
	// if( nDevices == 0 )
	// {
	// 	fprintf( stdout, ".........no devices detected.\n" );
	// 	goto PROGRAM_COMPLETE;
	// }
	// ApplicationDetermineCollectionParameters( nDevices, pdtDevices, &dtSettings );

	fprintf( stdout, "...OptotrakSetProcessingFlags\n" );
    if( OptotrakSetProcessingFlags( OPTO_LIB_POLL_REAL_DATA |
                                    OPTO_CONVERT_ON_HOST |
                                    OPTO_RIGID_ON_HOST ) )
    {
        goto ERROR_EXIT;
    }
	fprintf( stdout, "...OptotrakLoadCameraParameters\n" );
    if( OptotrakLoadCameraParameters( "standard" ) != OPTO_NO_ERROR_CODE )
	{
		goto ERROR_EXIT;
	}
    fprintf( stdout, "...OptotrakSetStroberPortTable\n" );
    if( OptotrakSetStroberPortTable( dtSettings.nMarkers - 1, 0, 0, 0 ) != OPTO_NO_ERROR_CODE )
	{
		goto ERROR_EXIT;
	} /* if */
	if( dtSettings.nMarkers == 0 )
	{
		fprintf( stdout, "Error: There are no markers to be activated.\n" );
		goto ERROR_EXIT;
	}
	fprintf( stdout, "...OptotrakSetupCollection\n" );
	fprintf( stdout, ".....%d, %.2f, %.0f, %d, %d, %d, %.2f, %.2f, %.0f, %.0f\n",
								 dtSettings.nMarkers - 1,
			                     dtSettings.fFrameFrequency,
				                 dtSettings.fMarkerFrequency,
					             dtSettings.nThreshold,
						         dtSettings.nMinimumGain,
							     dtSettings.nStreamData,
								 dtSettings.fDutyCycle,
								 dtSettings.fVoltage,
								 dtSettings.fCollectionTime,
								 dtSettings.fPreTriggerTime );
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
        goto ERROR_EXIT;
    }

    sleep( 1 );

	fprintf( stdout, "...OptotrakActivateMarkers\n" );
    if( OptotrakActivateMarkers( ) != OPTO_NO_ERROR_CODE )
    {
        goto ERROR_EXIT;
    }
	sleep( 1 );



    /*
     * Get frame of 3D data.
     */

	p3dData = (Position3d*)malloc( dtSettings.nMarkers * sizeof( Position3d ) );

    for( nCurFrame = 0; nCurFrame < 1; nCurFrame++ )
    {
        fprintf( stdout, "\n" );
        if( DataGetLatest3D( &uFrameNumber, &uElements, &uFlags, p3dData ) )
        {
            goto ERROR_EXIT;
        }

        fprintf( stdout, "Frame Number: %8u\n", uFrameNumber );
        fprintf( stdout, "Elements    : %8u\n", uElements );
        fprintf( stdout, "Flags       : 0x%04x\n", uFlags );
        for( nCurMarker = 0; nCurMarker < dtSettings.nMarkers; nCurMarker++ )
        {
			DisplayMarker( nCurMarker + 1, p3dData[nCurMarker] );
        } 


    }
    sleep(10);
	/*
	 * Stop the collection.
	 */

    if( OptotrakDeActivateMarkers() )
    {
        goto ERROR_EXIT;
    } 
	fprintf( stdout, "...OptotrakStopCollection\n" );
	if( OptotrakStopCollection( ) != OPTO_NO_ERROR_CODE )
	{
		goto ERROR_EXIT;
	} 

PROGRAM_COMPLETE:
    /*
     * CLEANUP
     */
	fprintf( stdout, "\n" );
	fprintf( stdout, "...TransputerShutdownSystem\n" );
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
	fprintf( stdout, "\nAn error has occurred during execution of the program.\n" );
    if( OptotrakGetErrorString( szNDErrorString, MAX_ERROR_STRING_LENGTH + 1 ) == 0 )
    {
        fprintf( stdout, szNDErrorString );
    }

	fprintf( stdout, "\n\n...TransputerShutdownSystem\n" );
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

