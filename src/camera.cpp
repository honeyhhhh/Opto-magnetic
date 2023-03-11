#include "camera.hpp"
#include "serial.hpp"
#include "bmp.hpp"
#include "threadpool.h"
#include <memory>


MyCamera::MyCamera()
{

}

void MyCamera::Init(string camip)
{
    cam_ip = camip;
    event_on = false;

    auto t0 = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
    cout << "init camera :" << cam_ip << " at " << t0 << endl; // cameras ticks same ? set min?

    CTlFactory& TlFactory = CTlFactory::GetInstance();
    CDeviceInfo di;
    di.SetIpAddress(cam_ip.c_str());

    camera.Attach(TlFactory.CreateDevice(di));
    pHandler1 = new CSampleCameraEventHandler(camip);
    pHandler2 = new CSampleImageEventHandler(camip);
    // camera.GrabCameraEvents = true;

    camera.Open();

    Sleep(50);



    camera.ExposureTimeAbs.TrySetValue(10000.0);
    camera.AcquisitionFrameRateEnable.SetValue(false);
    // camera.AcquisitionFrameRateAbs.SetValue(40.0);
    camera.Width.SetValue(1280);
    camera.Height.SetValue(1024);
    // camera.TriggerMode.SetValue(TriggerMode_Off);

    camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
    camera.TriggerMode.SetValue(TriggerMode_On);
    camera.TriggerSource.SetValue(TriggerSource_Line1);

    if (event_on)
    {

        camera.RegisterImageEventHandler( pHandler2, RegistrationMode_Append, Cleanup_Delete );

        camera.RegisterCameraEventHandler( pHandler1, "ExposureEndEventData", eMyExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_None );
        // camera.RegisterCameraEventHandler( pHandler1, "FrameStartEventTimestamp", eMyFrameStartEvent, RegistrationMode_Append, Cleanup_None );
        // camera.RegisterCameraEventHandler( pHandler1, "AcquisitionStartEventTimestamp", eMyAcquisitionStartEvent, RegistrationMode_Append, Cleanup_None );

        if (camera.EventSelector.TrySetValue( EventSelector_ExposureEnd ))
        {   // Enable it.
            if (!camera.EventNotification.TrySetValue( EventNotification_On ))
            {
                    // scout-f, scout-g, and aviator GigE cameras use a different value.
                camera.EventNotification.SetValue( EventNotification_GenICamEvent );
            }
        }
        // if (camera.EventSelector.TrySetValue( EventSelector_FrameStart ))
        //     {   // Enable it.
        //         if (!camera.EventNotification.TrySetValue( EventNotification_On ))
        //         {
        //                 // scout-f, scout-g, and aviator GigE cameras use a different value.
        //             camera.EventNotification.SetValue( EventNotification_GenICamEvent );
        //         }
        //     }
        // if (camera.EventSelector.TrySetValue( EventSelector_AcquisitionStart ))
        //     {   // Enable it.
        //         if (!camera.EventNotification.TrySetValue( EventNotification_On ))
        //         {
        //                 // scout-f, scout-g, and aviator GigE cameras use a different value.
        //             camera.EventNotification.SetValue( EventNotification_GenICamEvent );
        //         }
        //     }
    }

    camera.MaxNumBuffer = 5;
    camera.OutputQueueSize = 5;

    std::cout << "done" << std::endl;
}


MyCamera::~MyCamera()
{
    delete pHandler1;
    delete pHandler2;
    if (event_on)
    {
        camera.EventSelector.SetValue( EventSelector_ExposureEnd );
        camera.EventNotification.SetValue( EventNotification_Off );
        camera.EventSelector.SetValue( EventSelector_FrameStart );
        camera.EventNotification.SetValue( EventNotification_Off );
    }

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
    time_differ <<
    endl;
}

uint64_t MyCamera::Time_convert(uint64_t inter_ticks)
{
    uint64_t t = inter_ticks / 1000000;
    return t + this->time_differ;
}





unsigned __stdcall cam_get_frame1(LPVOID c)
{
    MyCamera *camera = (MyCamera *)c;
    std::ofstream fs((std::string(std::string("../" + camera->cam_ip + "/") + "time.txt")).c_str(), std::ios::out);
    std::ofstream fn((std::string(std::string("../" + camera->cam_ip + "/") + "num.txt")).c_str(), std::ios::out);

    int nbuff = 0;
    size_t bufferSize = 1024 * 1280;
    int width = 1280;
    int height = 1024;

    std::threadpool executor {16};

    CBaslerUniversalGrabResultPtr ptrGrabResult;
    
    std::vector<uint64_t> cam_Time(500);
    std::vector<unsigned long> frame_Number(500);
    std::vector<std::vector<uint8_t> > cam_Data (200, std::vector<uint8_t>(bufferSize));

    // sync clock
    PVOID pt = &sb_cam;
    auto barrier_t = (PSYNCHRONIZATION_BARRIER)pt;
    ::EnterSynchronizationBarrier(barrier_t, SYNCHRONIZATION_BARRIER_FLAGS_SPIN_ONLY);
    camera->camera.GevTimestampControlReset();
    Sleep(50);
    auto tt1 = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
    camera->camera.GevTimestampControlLatch.Execute();
    auto tt2 = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
    auto tt = camera->camera.GevTimestampValue.GetValue() / 1000000;
    auto tt3 = (tt1 + tt2) / 2; 
    camera->time_differ = tt3 - tt;
    cout << camera->time_differ << endl;

    camera->camera.StartGrabbing(10, GrabStrategy_UpcomingImage);//GrabStrategy_UpcomingImage
    Sleep(50);

    // 进入屏障 sync grab
	std::cout << camera->cam_ip << " thread into barrier" << std::endl;
    PVOID p = &sb;
    auto barrier = (PSYNCHRONIZATION_BARRIER)p;
    ::EnterSynchronizationBarrier(barrier, SYNCHRONIZATION_BARRIER_FLAGS_SPIN_ONLY);
	std::cout << camera->cam_ip << " thread start !" << std::endl;


    auto t1 = std::chrono::system_clock::now();

    while (camera->camera.IsGrabbing())
    {
        camera->camera.RetrieveResult( 10000, ptrGrabResult, TimeoutHandling_Return );
        if (ptrGrabResult->GrabSucceeded())
        {
            uint64_t f = camera->Time_convert(ptrGrabResult->GetTimeStamp()); //exposure start
            uint64_t fn = ptrGrabResult->GetImageNumber();
            uint64_t img_size = ptrGrabResult->GetImageSize();
            uint64_t nskip = ptrGrabResult->GetNumberOfSkippedImages();
            auto ff = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
            fs << f << " " << ff << " " << nskip << "\n";
            const uint8_t* pImageBuffer = (uint8_t*) ptrGrabResult->GetBuffer();


            std::unique_ptr<uint8_t[]> p = std::make_unique<uint8_t[]>(img_size);
            std::copy(pImageBuffer, pImageBuffer + img_size, p.get());
            
            executor.commit(SaveVectorAsBmp, std::move(p), width, height, 1, std::string(std::string("../" + camera->cam_ip + "/") + to_string(fn) + ".bmp"));
        }

		if (WaitForSingleObject(gDoneEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}
    }

    auto t2 = std::chrono::system_clock::now();
    // save
	// std::cout << "size" << cam_Time.size() << "\t" << cam_Data.size() << std::endl;
    // for (auto t : cam_Time)
	// 	fs << t << "\n";

    // int i = 0;
    // for (auto t : cam_Data)
    // {
    //     bmpEncoder(std::string(std::string("../" + camera->cam_ip + "/") + to_string(i) + ".bmp"), 1280, 1024, t, false);
    //     i++;
    // }


    // auto t3 = std::chrono::system_clock::now();

    std::cout << "ip " << camera->cam_ip << "\t";
    std::cout << "times :" << t1.time_since_epoch().count() << "~";
    std::cout << t2.time_since_epoch().count() << std::endl;

    fs.close();
    fn.close();
    camera->pHandler1->fss.close();
    camera->pHandler2->fs.close();

    camera->camera.StopGrabbing();
    _endthreadex(0);
    return 0;
}





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

                // CImagePersistence::Save(ImageFileFormat_Bmp, 
                //                     String_t((std::string(std::string("../" + camip + "/") + to_string(f) + ".bmp")).c_str()),
                //                     ptrGrabResult);