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

            // Print the model name of the camera.
            cout << "Using device " << cameras[i].GetDeviceInfo().GetModelName() << endl;
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
        const char* reply = ndiCommand(device, "VER:4");
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


int certus_test()
{
    char
        szNDErrorString[MAX_ERROR_STRING_LENGTH + 1];
    int
		i,
		nDevices;
	ApplicationDeviceInformation
		*pdtDevices;
    DeviceHandle
        *pdtDeviceHandles;
    DeviceHandleInfo
        *pdtDeviceHandlesInfo;


    /*
     * initialization
	 * intialize variables
     */
	pdtDevices = NULL;
	pdtDeviceHandles = NULL;
	pdtDeviceHandlesInfo = NULL;
	nDevices = 0;

	/*
	 * Announce that the program has started
	 */
	fprintf( stdout, "\nOptotrak Certus sample program #12\n\n" );

	/*
	 * look for the -nodld parameter that indicates 'no download'
	 */
	if(1)
	{
		/*
		 * Load the system of processors.
		 */
		fprintf( stdout, "...TransputerLoadSystem\n" );
		if( TransputerLoadSystem( "system" ) != OPTO_NO_ERROR_CODE )
		{
			goto ERROR_EXIT;
		} /* if */

		sleep( 1 );
	} /* if */

    /*
     * Communication Initialization
     * Once the system processors have been loaded, the application
     * prepares for communication by initializing the system processors.
     */
	fprintf( stdout, "...TransputerInitializeSystem\n" );
    if( TransputerInitializeSystem( OPTO_LOG_ERRORS_FLAG | OPTO_LOG_MESSAGES_FLAG ) != OPTO_NO_ERROR_CODE )
	{
		goto ERROR_EXIT;
	} /* if */

	/*
	 * Determine if this sample will run with the system attached.
	 * This sample is intended for Optotrak Certus systems.
	 */
	fprintf( stdout, "...DetermineSystem\n" );
	if( uDetermineSystem( ) != OPTOTRAK_CERTUS_FLAG )
	{
		goto PROGRAM_COMPLETE;
	} /* if */

    /*
     * Strober Initialization
     * Once communication has been initialized, the application must
     * determine the strober configuration.
     * The application retrieves device handles and all strober
     * properties from the system.
     */
	fprintf( stdout, "...DetermineStroberConfiguration\n" );
	if( DetermineStroberConfiguration( &pdtDeviceHandles, &pdtDeviceHandlesInfo, &nDevices ) != OPTO_NO_ERROR_CODE )
	{
		goto ERROR_EXIT;
	} /* if */

	/*
	 * check if any devices have been detected by the system
	 */
	if( nDevices == 0 )
	{
		fprintf( stdout, ".........no devices detected.  Quitting program...\n" );
		goto PROGRAM_COMPLETE;
	} /* if */

    /*
     * Now that all the device handles have been completely set up,
     * the application can store all the device handle information in
     * an internal data structure.  This will facilitate lookups when
     * a property setting needs to be checked.
     */
    ApplicationStoreDeviceProperties( &pdtDevices, pdtDeviceHandlesInfo, nDevices );


	/*
     * Turn the beeper ON
     */
	for( i = 0; i < nDevices; i++ )
	{
		if ( pdtDevices[i].nBeeper )
		{
			fprintf( stdout, "...OptotrakDeviceHandleSetBeeper (ON)\n" );
			if( OptotrakDeviceHandleSetBeeper( pdtDeviceHandlesInfo[i].pdtHandle->nID, BEEPER_STATE_ON ) != OPTO_NO_ERROR_CODE )
			{
				goto ERROR_EXIT;
			} /* if */

			/*
			 * Wait 1 second
			 */
			fprintf( stdout, "...beeping for one second\n" );
			sleep( 1 );

			/*
			 * Turn the beeper OFF
			 */
			fprintf( stdout, "...OptotrakDeviceHandleSetBeeper (OFF)\n" );
			if( OptotrakDeviceHandleSetBeeper( pdtDeviceHandlesInfo[i].pdtHandle->nID, BEEPER_STATE_OFF ) != OPTO_NO_ERROR_CODE )
			{
				goto ERROR_EXIT;
			} /* if */
		} /* if */
		else
		{
			fprintf( stdout, "...No beeper available on Device %.3d (ID %d)", (i + 1), pdtDeviceHandlesInfo[i].pdtHandle->nID );
		} /* else */
	} /* for */


PROGRAM_COMPLETE:
    /*
     * CLEANUP
     */
	fprintf( stdout, "\n" );
	fprintf( stdout, "...TransputerShutdownSystem\n" );
    OptotrakDeActivateMarkers( );
    TransputerShutdownSystem( );

    sleep(5);
	return 0;


ERROR_EXIT:
	/*
	 * Indicate that an error has occurred
	 */
	fprintf( stdout, "\nAn error has occurred during execution of the program.\n" );
    if( OptotrakGetErrorString( szNDErrorString, MAX_ERROR_STRING_LENGTH + 1 ) == 0 )
    {
        fprintf( stdout, szNDErrorString );
    } /* if */

	fprintf( stdout, "\n\n...TransputerShutdownSystem\n" );
	OptotrakDeActivateMarkers( );
	TransputerShutdownSystem( );
    sleep(5);

    return 1;

} 