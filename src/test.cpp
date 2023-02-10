#include "test.hpp"

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <chrono>
#include <iostream>

FILE *logg = NULL;

enum MyEvents
{
    eMyExposureEndEvent = 100,
    eMyFrameStartOvertrigger = 200,
    eMyFrameStartEvent = 300,
    eMyAcquisitionStartEvent = 400,
    eMyAcquisitionStartOvertriggerEvent = 500

    // More events can be added here.
};

class CCameraEventPrinter : public CCameraEventHandler
{
public:
    virtual void OnCameraEvent( CInstantCamera& camera, intptr_t userProvidedId, GenApi::INode* pNode )
    {
        std::cout << "OnCameraEvent event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;
        std::cout << "User provided ID: " << userProvidedId << std::endl;
        std::cout << "Event data node name: " << pNode->GetName() << std::endl;
        CParameter value( pNode );
        if (value.IsValid())
        {
            std::cout << "Event node data: " << value.ToString() << std::endl;
        }
        std::cout << std::endl;
    }
};

class CSampleCameraEventHandler : public CBaslerUniversalCameraEventHandler
{
public:
    // Only very short processing tasks should be performed by this method. Otherwise, the event notification will block the
    // processing of images.
    virtual void OnCameraEvent( CBaslerUniversalInstantCamera& camera, intptr_t userProvidedId, GenApi::INode* /* pNode */ )
    {
        // std::cout << std::endl;
        switch (userProvidedId)
        {
            case eMyExposureEndEvent: // Exposure End event
                cout << "Exposure End event. FrameID: " << camera.ExposureEndEventFrameID.GetValue() << " Timestamp: " << camera.ExposureEndEventTimestamp.GetValue() << std::endl;
                break;
            case eMyFrameStartOvertrigger:
                cout << "FrameStartOvertrigger event. Timestamp: " << camera.FrameStartOvertriggerEventTimestamp.GetValue() << std::endl;
                break;
            case eMyFrameStartEvent:
                cout << "FrameStart event. Timestamp: " << camera.FrameStartEventTimestamp.GetValue() << std::endl;
                break;
            case eMyAcquisitionStartEvent:
                cout << "AcquisitionStart event. Timestamp: " << camera.AcquisitionStartEventTimestamp.GetValue() << std::endl;
                break;
            case eMyAcquisitionStartOvertriggerEvent:
                cout << "AcquisitionStartOvertrigger Event Timestamp: " << camera.AcquisitionStartOvertriggerEventTimestamp.GetValue() << std::endl;
                break;
        }
    }
};
//Example of an image event handler.
class CSampleImageEventHandler : public CBaslerUniversalImageEventHandler
{
public:
    virtual void OnImageGrabbed( CBaslerUniversalInstantCamera& camera, const CBaslerUniversalGrabResultPtr& ptrGrabResult )
    {
        if (ptrGrabResult->GrabSucceeded())
        {
            cout << "ImageNumber: " << ptrGrabResult->GetImageNumber() << "\tExposure start ts: " << ptrGrabResult->GetTimeStamp() << " " << ptrGrabResult->GetNumberOfSkippedImages() << endl;
            if (ptrGrabResult->ChunkTimestamp.IsReadable())
                cout << "OnImageGrabbed: TimeStamp (Result) : " << ptrGrabResult->ChunkTimestamp.GetValue() << endl;
            if (ptrGrabResult->ChunkExposureTime.IsReadable())
                cout << "OnImageGrabbed: ExposureTime (Result) : " << ptrGrabResult->ChunkExposureTime.GetValue() << endl;
            if (ptrGrabResult->ChunkFramecounter.IsReadable()) // frame start /  at 0
                cout << "OnImageGrabbed: Framecouter (Result) : " << ptrGrabResult->ChunkFramecounter.GetValue() << endl;
            if (ptrGrabResult->ChunkTriggerinputcounter.IsReadable())
                cout << "OnImageGrabbed: Triggerinputcounter (Result) : " << ptrGrabResult->ChunkTriggerinputcounter.GetValue() << endl;

            
        }
    }
};

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
        CInstantCameraArray cameras( min( devices.size(), 5 ) );
        
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
        for (uint32_t i = 0; i < 10 && cameras.IsGrabbing(); ++i)
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
                // cout << "BufferSize: " << ptrGrabResult->GetBufferSize() << endl; //size of image
                cout << "?" << ptrGrabResult->GetTimeStamp() << endl;
                cout << "?" << ptrGrabResult->GetImageNumber() << endl;
                // cout << "?" << ptrGrabResult->GetImageSize() << endl;
                cout << "?" << ptrGrabResult->GetNumberOfSkippedImages() << endl;
                cout << "?" << ptrGrabResult->IsChunkDataAvailable() << endl;
                // cout << "?" << ptrGrabResult->GetPayloadSize() << endl; //size  of image
                cout << "?" << ptrGrabResult->GetID() << endl;
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
    // sleep(10);
    // std::cout is not use
    // logg = fopen("../log_kk.txt", "w");
    // if (logg == NULL)
    // {
    //     std::cout << "failed create log" << endl;
    // }
    // fprintf(logg, "begin basler test\n");

    PylonInitialize();

    CTlFactory& TlFactory = CTlFactory::GetInstance();
    CDeviceInfo di;
    di.SetIpAddress( "169.254.0.55");
    CBaslerUniversalInstantCamera camera( TlFactory.CreateDevice( di ) );
    



    // Create an example event handler. In the present case, we use one single camera handler for handling multiple camera events.
    // The handler prints a message for each received event.
    CSampleCameraEventHandler* pHandler1 = new CSampleCameraEventHandler;
    // Create another more generic event handler printing out information about the node for which an event callback
    // is fired.
    // CCameraEventPrinter* pHandler2 = new CCameraEventPrinter;    // For demonstration purposes only, register another image event handler.
    camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete );
    // Camera event processing must be activated first, the default is off.
    camera.GrabCameraEvents = true;




    
    camera.Open(); //打开才能读参数
    // Check if the device supports events.
    if (!camera.EventSelector.IsWritable())
    {
            throw RUNTIME_EXCEPTION( "The device doesn't support events." );
    }
    camera.ExposureTimeAbs.TrySetValue(5000.0);
    camera.AcquisitionFrameRateEnable.SetValue(true);
    camera.AcquisitionFrameRateAbs.SetValue(40.0);
    camera.Width.SetValue(1280);
    camera.Height.SetValue(1024);
    camera.GevSCPSPacketSize.SetValue(8192); //巨型帧
    camera.TriggerMode.SetValue(TriggerMode_Off);
    // camera.TriggerSource.SetValue(TriggerSource_Software);


    // camera.MaxNumQueuedBuffer.SetToMinimum();
    // camera.MaxNumGrabResults.SetToMinimum();
    // camera.MaxNumBuffer.SetToMaximum();
    // camera.OutputQueueSize = 1;
    camera.RegisterCameraEventHandler( pHandler1, "ExposureEndEventData", eMyExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_None );
    // camera.RegisterCameraEventHandler( pHandler1, "FrameStartOvertriggerEventTimestamp", eMyFrameStartOvertrigger, RegistrationMode_Append, Cleanup_None );
    camera.RegisterCameraEventHandler( pHandler1, "FrameStartEventTimestamp", eMyFrameStartEvent, RegistrationMode_Append, Cleanup_None );
    camera.RegisterCameraEventHandler( pHandler1, "AcquisitionStartEventTimestamp", eMyAcquisitionStartEvent, RegistrationMode_Append, Cleanup_None );
    // camera.RegisterCameraEventHandler( pHandler1, "AcquisitionStartOvertriggerEventTimestamp", eMyAcquisitionStartOvertriggerEvent, RegistrationMode_Append, Cleanup_None );
    
    // camera.RegisterCameraEventHandler( pHandler2, "ExposureEndEventFrameID", eMyExposureEndEvent, RegistrationMode_Append, Cleanup_None );
    // camera.RegisterCameraEventHandler( pHandler2, "ExposureEndEventTimestamp", eMyExposureEndEvent, RegistrationMode_Append, Cleanup_None );
    




    // camera.ChunkModeActive.SetValue(true);
    // camera.ChunkSelector.SetValue(ChunkSelector_ExposureTime);
    // camera.ChunkEnable.SetValue(true);
    // camera.ChunkSelector.SetValue(ChunkSelector_Timestamp);
    // camera.ChunkEnable.SetValue(true);
    // camera.ChunkSelector.SetValue(ChunkSelector_Framecounter);
    // camera.ChunkEnable.SetValue(true);
    // camera.ChunkSelector.SetValue(ChunkSelector_Triggerinputcounter);
    // camera.ChunkEnable.SetValue(true);




    cout << "Using device " << 
    camera.GetDeviceInfo().GetModelName() << " " <<
    camera.GetDeviceInfo().GetIpAddress() << " " <<
    camera.GetDeviceInfo().GetDeviceClass() <<  " " <<
    camera.GetDeviceInfo().GetSerialNumber() << " " <<
    CParameter(camera.GetNodeMap().GetNode("OffsetX")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("OffsetY")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ExposureTimeAbs")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ExposureTimeRaw")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ExposureTimeBaseAbs")).IsValid() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ExposureMode")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("AcquisitionMode")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("AcquisitionFrameRateEnable")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("AcquisitionFrameRateAbs")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("Width")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("Height")).ToString() << " " <<
    CEnumParameter(camera.GetNodeMap().GetNode("PixelFormat")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("TriggerSource")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("TriggerActivation")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("TriggerSelector")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("TriggerMode")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ReadoutTimeAbs")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ResultingFrameRateAbs")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ShutterMode")).ToString() << " " <<
    camera.MaxNumBuffer.GetValue() << " " <<
    camera.NumReadyBuffers.GetValue() << " " <<
    camera.NumEmptyBuffers.GetValue() << " " <<
    camera.MaxNumGrabResults.GetValue() << " " <<
    camera.MaxNumQueuedBuffer.GetValue() << " " <<
    camera.OutputQueueSize.GetValue() << " " << endl;

    //init: Using device acA1300-60gmNIR 169.254.0.55 BaslerGigE 21752969 1 1 5000 Timed Continuous 0 10 1280 1024 Mono8 Line1 RisingEdge FrameStart Off 14705 68.0041 Global 
    //set/: Using device acA1300-60gmNIR 169.254.0.55 BaslerGigE 21752969 1 1 5000 Timed Continuous 1 40 1280 1024 Mono8 Line1 RisingEdge FrameStart Off 14705 39.9904 Global 

    // cout << camera.GevTimestampTickFrequency() << endl; //1000000000
    // fprintf(logg, "\nstart grabbing(system): %llu\n\n", std::chrono::system_clock::now().time_since_epoch().count());




    if (camera.EventSelector.TrySetValue( EventSelector_ExposureEnd ))
    {   // Enable it.
        if (!camera.EventNotification.TrySetValue( EventNotification_On ))
        {
                // scout-f, scout-g, and aviator GigE cameras use a different value.
            camera.EventNotification.SetValue( EventNotification_GenICamEvent );
        }
    }
   if (camera.EventSelector.TrySetValue( EventSelector_FrameStart ))
    {   // Enable it.
        if (!camera.EventNotification.TrySetValue( EventNotification_On ))
        {
                // scout-f, scout-g, and aviator GigE cameras use a different value.
            camera.EventNotification.SetValue( EventNotification_GenICamEvent );
        }
    }
   if (camera.EventSelector.TrySetValue( EventSelector_AcquisitionStart ))
    {   // Enable it.
        if (!camera.EventNotification.TrySetValue( EventNotification_On ))
        {
                // scout-f, scout-g, and aviator GigE cameras use a different value.
            camera.EventNotification.SetValue( EventNotification_GenICamEvent );
        }
    }
    camera.MaxNumBuffer = 15;
    camera.OutputQueueSize = 10;
    camera.StartGrabbing(GrabStrategy_OneByOne);
    /*
        if busy after startgrabbing for a long while
        GrabStrategy_OneByOne: The images are processed in the order of their arrival.  the grab engine thread retrieves the image data and queues the buffers into the internal output queue.           
        (SkippedImages = 0)     All triggered images are still waiting in the output queue and are now retrieved. 
        (NumReadyBuffers = Max) The grabbing continues in the background, e.g. when using hardware trigger mode, as long as the grab engine does not run out of buffers.
        (only read full grab buffer can queue new image from output queue, so may cause incontinuity

        GrabStrategy_LatestImageOnly: The images are processed in the order of their arrival but only the last received image is kept in the output queue.
        (SkippedImages = ? at num 1)  This strategy can be useful when the acquired images are only displayed on the screen.
        (SkippedImages = 0 af      ) If the processor has been busy for a while and images could not be displayed automatically
        (continuity)                  the latest image is displayed when processing time is available again.
        (NumReadyBuffers = 0)         Only the last received image is waiting in the internal output queue and is now retrieved.
                                      The grabbing continues in the background, e.g. when using the hardware trigger mode.
        Imagenum 1 is the latest in buffer , retrieved block less than 25ms


        GrabStrategy_LatestImages     The images are processed
        (camera.OutputQueueSize)      in the order of their arrival, but only a number of the images received last are kept in the output queue.
        (SkippedImages = ? at num 1)  Only the images received last are waiting in the internal output queue and are now retrieved.
        (NumReadyBuffers = OutputQueueSize --  retrieved block less than 25ms)
        (continuity)                  
        Imagenum 1 is the latest - OutputQueueSize 

        GrabStrategy_UpcomingImage   the buffer queue is kept empty / real latest
                                    A buffer for grabbing is queued each time when RetrieveResult() is called. The image data is grabbed into the buffer and returned.
                                    This ensures that the image grabbed is the next image received from the camera.  All images are still transported to the computer.

                                    
    */


    // camera.GevTimestampControlLatch.Execute();
    // cout << "start grab : " << camera.GevTimestampValue.GetValue() << endl;
    // cout << std::chrono::system_clock::now().time_since_epoch().count() << endl; //1675776932585
    // camera.GevTimestampControlLatch.Execute();
    // cout << camera.GevTimestampValue.GetValue() << endl; //after StartGrabbing 4867797
    // cout << std::chrono::system_clock::now().time_since_epoch().count() << endl; //1675776932588
    // fprintf(logg, "after start grabbing(camera): %lld\n", camera.GevTimestampValue.GetValue());
    // fprintf(logg, "after start grabbing(system): %llu\n", std::chrono::system_clock::now().time_since_epoch().count());

    // This smart pointer will receive the grab result data.


    WaitObject::Sleep(5 * 1000);


    CBaslerUniversalGrabResultPtr ptrGrabResult;

    // for (int i = 0; i < 1; i++)
    // {
    //     // Execute the software trigger. Wait up to 1000 ms for the camera to be ready for trigger.
    //     if (camera.WaitForFrameTriggerReady( 1000, TimeoutHandling_ThrowException ))
    //     {
    //             camera.ExecuteSoftwareTrigger();
    //     }
    // }
    // if (camera.GetGrabResultWaitObject().Wait( 0 ))
    // {
    //     cout << endl << "Grab results wait in the output queue." << endl << endl;
    // }
        // Retrieve grab results and notify the camera event and image event handlers.
        // camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException );
        // camera.GevTimestampControlLatch.Execute(); 
        // cout << "after RetrieveResult: " << camera.GevTimestampValue.GetValue() << endl; 
        // cout << "ImageNumber: " << ptrGrabResult->GetImageNumber() << "\tExposure start ts: " << ptrGrabResult->GetTimeStamp() << endl;

        
    int nBuffersInQueue = 0;
    while (camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_Return ))
    {
        if (nBuffersInQueue == 11)
        {
            cout << camera.NumReadyBuffers.GetValue() << endl;
        }
        if (nBuffersInQueue == 20)
        {
            cout << camera.NumReadyBuffers.GetValue() << endl;
            break;
        }
        nBuffersInQueue++;
        camera.GevTimestampControlLatch.Execute(); 
        cout << "after RetrieveResult: " << camera.GevTimestampValue.GetValue() / 1000000 << endl;
        auto t1 = std::chrono::system_clock::now().time_since_epoch().count();
        cout << t1 / 10000 << endl;;
    }
    cout << "Retrieved " << nBuffersInQueue << " grab results from output queue." << endl << endl;
        

    
    // CGrabResultPtr ptrGrabResult;

    // for (uint32_t i = 0; i < 5 && camera.IsGrabbing(); ++i)
    // {
    //     // fprintf(logg, "number of buffer ready: %lld\t%llu\t%llu\n",camera.NumReadyBuffers.GetValue(), camera.MaxNumBuffer.GetValue(),camera.NumEmptyBuffers.GetValue());

    //     // cout << "1 " << std::chrono::system_clock::now().time_since_epoch().count() << endl; 
    //     camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException ); // 阻塞
    //     camera.GevTimestampControlLatch.Execute(); 
    //     cout << "after RetrieveResult: " << camera.GevTimestampValue.GetValue() << endl; //after RetrieveResult: 4867864  4867889 4867914 4867939
    //     //                                                                                  //when Exposure start : 4867799  4867824 4867849 4867874
    //     // cout << "system " << std::chrono::system_clock::now().time_since_epoch().count() << endl; //1675776932656 1675776932681 1675776932706 1675776932731
    //     // fprintf(logg, "after RetrieveResult(camera): %lld\n", camera.GevTimestampValue.GetValue());
    //     // fprintf(logg, "after RetrieveResult(system): %llu\n", std::chrono::system_clock::now().time_since_epoch().count());


    //     if (ptrGrabResult->GrabSucceeded())
    //     {
    //         intptr_t cameraContextValue = ptrGrabResult->GetCameraContext();
    //         // cout << "Camera " << cameraContextValue << ": " << camera.GetDeviceInfo().GetModelName() << endl;
    //         // cout << "BufferSize: " << ptrGrabResult->GetBufferSize() << endl; //size of image ptrGrabResult->GetImageSize() ptrGrabResult->GetPayloadSize()
    //         cout << "ImageNumber: " << ptrGrabResult->GetImageNumber() << "\tExposure start ts: " << ptrGrabResult->GetTimeStamp() << endl;
    //         // fprintf(logg, "Imagenum: %lld\tExposure start ts: %lld\tNumberOfSkippedImages :%lld\n", ptrGrabResult->GetImageNumber(), ptrGrabResult->GetTimeStamp(), ptrGrabResult->GetNumberOfSkippedImages());
            
    //         // const uint8_t* pImageBuffer = (uint8_t*) ptrGrabResult->GetBuffer();
    //         // cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;

    //         // cout << "Exposure start ts: " << ptrGrabResult->ChunkTimestamp.GetValue() << endl;
    //         // cout << "Framecount: " << ptrGrabResult->ChunkFramecounter.GetValue() << endl;
    //         // cout << "Trigger input: " << ptrGrabResult->ChunkTriggerinputcounter.GetValue() << endl;
    //         // fprintf(logg, "Framecount: %lld\tTrigger inputs: %lld\n", ptrGrabResult->ChunkFramecounter.GetValue(), ptrGrabResult->ChunkTriggerinputcounter.GetValue());


    //         // auto t1 = std::chrono::steady_clock::now();
    //         // CImagePersistence::Save(ImageFileFormat_Bmp, 
    //         //                         String_t(std::string("../k5/" + to_string(ptrGrabResult->GetImageNumber()) + ".bmp").c_str()),
    //         //                         ptrGrabResult);
    //         // auto t2 = std::chrono::steady_clock::now();
    //         // double dr_ms = std::chrono::duration<double,std::milli>(t2-t1).count();
    //         // // cout << "save time :" << dr_ms << endl;
    //         // fprintf(logg, "save using time :%llf\n", dr_ms);


            // int nBuffersInQueue = 0;
            // while (camera.RetrieveResult( 0, ptrGrabResult, TimeoutHandling_Return ))
            // {
            //     nBuffersInQueue++;
            // }
            // cout << "Retrieved " << nBuffersInQueue << " grab results from output queue." << endl << endl;

            

    //     }
    //     else
    //     {
    //         cout << "Error: " << std::hex << ptrGrabResult->GetErrorCode() << std::dec << " " << ptrGrabResult->GetErrorDescription() << endl;
    //     }
    // }

    camera.EventSelector.SetValue( EventSelector_ExposureEnd );
    camera.EventNotification.SetValue( EventNotification_Off );

    camera.StopGrabbing();
    cout << "close" << endl;

    camera.Close();
    fclose(logg);

    PylonTerminate();
    // cerr << endl << " Press enter to exit." << endl;
    // while (cin.get() != '\n');
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
	sleep( 1);

    cerr << endl << "Activate Markers.. Press enter to exit." << endl;
    while (cin.get() != '\n');

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

