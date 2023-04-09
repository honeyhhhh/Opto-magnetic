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
typedef uint8_t BYTE;



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
    std::ofstream fss;
    // Only very short processing tasks should be performed by this method. Otherwise, the event notification will block the
    // processing of images.
    virtual void OnCameraEvent( CBaslerUniversalInstantCamera& camera, intptr_t userProvidedId, GenApi::INode* /* pNode */ )
    {
        // std::cout << std::endl;
        switch (userProvidedId)
        {
            case eMyExposureEndEvent: // Exposure End event
                fss << camera.ExposureEndEventFrameID.GetValue() << " " << camera.ExposureEndEventTimestamp.GetValue() << "\n";
                // std::cout << "Exposure End event. FrameID: " << camera.ExposureEndEventFrameID.GetValue() << " Timestamp: " << camera.ExposureEndEventTimestamp.GetValue() / 1000000 << std::endl;
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
    CSampleCameraEventHandler() {}

    CSampleCameraEventHandler(string ip)
    {
        fss.open((std::string(std::string("../" + ip + "/") + "exend_time.txt")).c_str(), std::ios::out);
    }
    ~CSampleCameraEventHandler()
    {

    }
};
//Example of an image event handler.
class CSampleImageEventHandler : public CBaslerUniversalImageEventHandler
{
public:
    std::ofstream fs;

    virtual void OnImageGrabbed( CBaslerUniversalInstantCamera& camera, const CBaslerUniversalGrabResultPtr& ptrGrabResult )
    {
        if (ptrGrabResult->GrabSucceeded())
        {
            fs << ptrGrabResult->GetTimeStamp() << " " << ptrGrabResult->GetNumberOfSkippedImages() << "\n";
            // cout << "camera : " << camera.GetDeviceInfo().GetIpAddress() << " ";
            // cout << "ImageNumber: " << ptrGrabResult->GetImageNumber() << "\tExposure start ts: " << ptrGrabResult->GetTimeStamp() / 1000000 << " " << ptrGrabResult->GetNumberOfSkippedImages() << endl;
            // if (ptrGrabResult->ChunkTimestamp.IsReadable())
            //     cout << "OnImageGrabbed: TimeStamp (Result) : " << ptrGrabResult->ChunkTimestamp.GetValue() << endl;
            // if (ptrGrabResult->ChunkExposureTime.IsReadable())
            //     cout << "OnImageGrabbed: ExposureTime (Result) : " << ptrGrabResult->ChunkExposureTime.GetValue() << endl;
            // if (ptrGrabResult->ChunkFramecounter.IsReadable()) // frame start /  at 0
            //     cout << "OnImageGrabbed: Framecouter (Result) : " << ptrGrabResult->ChunkFramecounter.GetValue() << endl;
            // if (ptrGrabResult->ChunkTriggerinputcounter.IsReadable())
            //     cout << "OnImageGrabbed: Triggerinputcounter (Result) : " << ptrGrabResult->ChunkTriggerinputcounter.GetValue() << endl;
        }
    }
    CSampleImageEventHandler() {}

    CSampleImageEventHandler(string ip)
    {
        fs.open((std::string(std::string("../" + ip + "/") + "exstart_time.txt")).c_str(), std::ios::out);
    }
    ~CSampleImageEventHandler()
    {
    }
};


class MyCamera
{
public:
    CBaslerUniversalInstantCamera camera;
    uint64_t time_differ; // us
    string cam_ip;
    bool event_on;
    int static_count;

    CSampleCameraEventHandler* pHandler1;
    CSampleImageEventHandler *pHandler2;


    MyCamera();
    void Init(string camip);
    ~MyCamera();
    void PrintConfig();
    uint64_t Time_convert(uint64_t inter_ticks); // from camera ticks to system ticks

};

unsigned __stdcall cam_get_frame1(LPVOID c);
void cam_get_frame2(MyCamera *camera);







#endif	// _CAMERA_H