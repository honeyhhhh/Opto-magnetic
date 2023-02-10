#ifndef _CAMERA_H 
#define _CAMERA_H 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>
#include <pylon/CameraEventHandler.h>
#ifdef PYLON_WIN_BUILD
    #include <pylon/PylonGUI.h>
#endif

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
 
    
    CSampleCameraEventHandler* pHandler1 = new CSampleCameraEventHandler;
    camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete );
    camera.GrabCameraEvents = true;

    camera.Open();

    camera.ExposureTimeAbs.TrySetValue(5000.0);
    camera.AcquisitionFrameRateEnable.SetValue(true);
    camera.AcquisitionFrameRateAbs.SetValue(40.0);
    camera.Width.SetValue(1280);
    camera.Height.SetValue(1024);
    camera.TriggerMode.SetValue(TriggerMode_Off);

    camera.RegisterCameraEventHandler( pHandler1, "ExposureEndEventData", eMyExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_None );
    camera.RegisterCameraEventHandler( pHandler1, "FrameStartEventTimestamp", eMyFrameStartEvent, RegistrationMode_Append, Cleanup_None );

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


    camera.MaxNumBuffer = 15;
    camera.OutputQueueSize = 10;

    auto t1 = std::chrono::system_clock::now().time_since_epoch().count();
    camera.GevTimestampControlLatch.Execute(); 
    auto tt = camera.GevTimestampValue.GetValue() / 1000000;
    auto t2 = std::chrono::system_clock::now().time_since_epoch().count();
    auto t3 = (t1 + t2) / 2 / 10000; 

    timediffer = t3 - tt;

    PrintConfig();

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