#ifndef _CAMERA_H 
#define _CAMERA_H 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>
#include <pylon/CameraEventHandler.h>
#ifdef PYLON_WIN_BUILD
    #include <pylon/PylonGUI.h>
#endif

#include "sync.hpp"
#include <vector>

using namespace std;
using namespace Pylon;
using namespace Basler_UniversalCameraParams;
static const uint32_t c_countOfImagesToGrab = 5;
static const size_t c_maxCamerasToUse = 4;

const string CamIps[4] = {"169.254.0.22", "169.254.0.55", "169.254.0.44", "169.254.0.33"};



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
                cout << "Exposure End event. FrameID: " << camera.ExposureEndEventFrameID.GetValue() << " Timestamp: " << camera.ExposureEndEventTimestamp.GetValue() / 1000000 << std::endl;
                break;
            case eMyFrameStartOvertrigger:
                cout << "FrameStartOvertrigger event. Timestamp: " << camera.FrameStartOvertriggerEventTimestamp.GetValue() / 1000000 << std::endl;
                break;
            case eMyFrameStartEvent:
                cout << "FrameStart event. Timestamp: " << camera.FrameStartEventTimestamp.GetValue() / 1000000 << std::endl;
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
            cout << "camera : " << camera.GetDeviceInfo().GetIpAddress() << " ";
            cout << "ImageNumber: " << ptrGrabResult->GetImageNumber() << "\tExposure start ts: " << ptrGrabResult->GetTimeStamp() / 1000000 << " " << ptrGrabResult->GetNumberOfSkippedImages() << endl;
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


class MyCamera
{
public:
    CBaslerUniversalInstantCamera camera;
    uint64_t timediffer; // us

    MyCamera(const string camip);
    ~MyCamera();
    void PrintConfig();
    uint64_t Time_convert(uint64_t inter_ticks); // from camera ticks to system ticks



};

MyCamera::MyCamera(const string camip)
{
    auto t0 = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
    cout << "init camera :" << camip << " at " << t0 << endl; // cameras ticks same ? set min?

    CTlFactory& TlFactory = CTlFactory::GetInstance();
    CDeviceInfo di;
    di.SetIpAddress(camip.c_str());

    camera.Attach(TlFactory.CreateDevice(di));

    
    // CSampleCameraEventHandler* pHandler1 = new CSampleCameraEventHandler;
    // camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete );
    // camera.GrabCameraEvents = true;

    camera.Open();

    auto t1 = std::chrono::system_clock::now().time_since_epoch().count();
    camera.GevTimestampControlLatch.Execute(); 
    auto t2 = std::chrono::system_clock::now().time_since_epoch().count();
    auto tt = camera.GevTimestampValue.GetValue() / 1000000;

    auto t3 = (t1 + t2) / 2 / 10000; 
    timediffer = t3 - tt;
    // cout << "camera :" << camip << " at " << t1/10000 << " " << t2/10000 << " in " << tt << endl;




    camera.ExposureTimeAbs.TrySetValue(5000.0);
    camera.AcquisitionFrameRateEnable.SetValue(true);
    camera.AcquisitionFrameRateAbs.SetValue(40.0);
    camera.Width.SetValue(1280);
    camera.Height.SetValue(1024);
    camera.TriggerMode.SetValue(TriggerMode_Off);
    // camera.TriggerSource.SetValue(TriggerSource_Software);

    // camera.RegisterCameraEventHandler( pHandler1, "ExposureEndEventData", eMyExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_None );
    // camera.RegisterCameraEventHandler( pHandler1, "FrameStartEventTimestamp", eMyFrameStartEvent, RegistrationMode_Append, Cleanup_None );
    // camera.RegisterCameraEventHandler( pHandler1, "AcquisitionStartEventTimestamp", eMyAcquisitionStartEvent, RegistrationMode_Append, Cleanup_None );

    // if (camera.EventSelector.TrySetValue( EventSelector_ExposureEnd ))
    // {   // Enable it.
    //     if (!camera.EventNotification.TrySetValue( EventNotification_On ))
    //     {
    //             // scout-f, scout-g, and aviator GigE cameras use a different value.
    //         camera.EventNotification.SetValue( EventNotification_GenICamEvent );
    //     }
    // }
//    if (camera.EventSelector.TrySetValue( EventSelector_FrameStart ))
//     {   // Enable it.
//         if (!camera.EventNotification.TrySetValue( EventNotification_On ))
//         {
//                 // scout-f, scout-g, and aviator GigE cameras use a different value.
//             camera.EventNotification.SetValue( EventNotification_GenICamEvent );
//         }
//     }
//    if (camera.EventSelector.TrySetValue( EventSelector_AcquisitionStart ))
//     {   // Enable it.
//         if (!camera.EventNotification.TrySetValue( EventNotification_On ))
//         {
//                 // scout-f, scout-g, and aviator GigE cameras use a different value.
//             camera.EventNotification.SetValue( EventNotification_GenICamEvent );
//         }
//     }

    camera.MaxNumBuffer = 15;
    camera.OutputQueueSize = 10;

    cout << "cam :" << camip << " ready" << endl;
    while (!cam_ready){ std::this_thread::sleep_for(std::chrono::microseconds(50));}


    // PrintConfig();
    // need sync here

    camera.StartGrabbing(GrabStrategy_UpcomingImage);
    CBaslerUniversalGrabResultPtr ptrGrabResult;

    auto t4 = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
    cout << "\t\t\tstart grab :" << camip << " at " << t4  << endl;

    // ----------------------
    int nbuff = 0;
    while (camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_Return ))
    {
        // auto t5 = std::chrono::system_clock::now().time_since_epoch().count();
        // cout << "camera : " << camera.GetDeviceInfo().GetIpAddress() << " ";
        // cout << "ImageNumber: " << ptrGrabResult->GetImageNumber() << "\tExposure start ts: " << Time_convert(ptrGrabResult->GetTimeStamp()) + 50 << " " << t5/10000 << " " << ptrGrabResult->GetNumberOfSkippedImages() << endl;

        uint64_t f = Time_convert(ptrGrabResult->GetTimeStamp()) + 50;
 
        if (ptrGrabResult->GrabSucceeded())
        {

            CImagePersistence::Save(ImageFileFormat_Bmp, 
                                    String_t((std::string(std::string("../" + camip + "/") + to_string(f) + ".bmp")).c_str()),
                                    ptrGrabResult);

        }

        nbuff++;
        if (nbuff == 40)
            break;
    }
    // ----------------------

    // for (int i = 0; i < 1; i++)
    // {
    //     // Execute the software trigger. Wait up to 1000 ms for the camera to be ready for trigger.
    //     if (camera.WaitForFrameTriggerReady( 1000, TimeoutHandling_ThrowException ))
    //     {
    //         camera.ExecuteSoftwareTrigger();
    //     }
    // }
    // camera.RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException );
    // cout << "ImageNumber: " << ptrGrabResult->GetImageNumber() << "\tExposure start ts: " << ptrGrabResult->GetTimeStamp() << endl;
    // --------------------------

    camera.StopGrabbing();

}

MyCamera::~MyCamera()
{
    camera.EventSelector.SetValue( EventSelector_ExposureEnd );
    camera.EventNotification.SetValue( EventNotification_Off );
    camera.EventSelector.SetValue( EventSelector_FrameStart );
    camera.EventNotification.SetValue( EventNotification_Off );

    camera.StopGrabbing();

    camera.Close();

}


void MyCamera::PrintConfig()
{
    cout << "Using device " << 
    camera.GetDeviceInfo().GetModelName() << " " <<
    camera.GetDeviceInfo().GetIpAddress() << " " <<
    camera.GetDeviceInfo().GetDeviceClass() <<  " " <<
    camera.GetDeviceInfo().GetSerialNumber() << " " <<
    CParameter(camera.GetNodeMap().GetNode("OffsetX")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("OffsetY")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ExposureTimeAbs")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("ExposureMode")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("AcquisitionMode")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("AcquisitionFrameRateEnable")).ToString() << "-" <<
    CParameter(camera.GetNodeMap().GetNode("AcquisitionFrameRateAbs")).ToString() << " " <<
    CParameter(camera.GetNodeMap().GetNode("Width")).ToString() << "-" <<
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
    camera.OutputQueueSize.GetValue() << " " <<
    timediffer <<
    endl;
}

uint64_t MyCamera::Time_convert(uint64_t inter_ticks)
{
    uint64_t t = inter_ticks / 1000000;
    return t + timediffer;
}


void Grab_sync(const string camip)
{
    MyCamera c(camip);
}







#endif	// _CAMERA_H