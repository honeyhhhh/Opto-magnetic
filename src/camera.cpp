#include "camera.hpp"
#include "serial.hpp"
#include "bmp.hpp"
#include <memory>
#include "threadpool.h"

class A 
{
public:
    static int Afun(int n = 0)
    {
        std::cout << n << std::endl;
        return 0;
    }

    static void SaveVectorAsBmpxx(std::string filename)
    {

    }


    static void SaveVectorAsBmpx(std::string filename, std::unique_ptr<uint8_t []> p)
    {
        std::cout << filename << std::endl;
    }

    static void SaveVectorAsBmpxxx(int width, int heigth, std::string filename, uint8_t *pp)
    {
        auto imgbuffer = pp;
        int channels = 1;
        std::ofstream out(filename, std::ios::binary);

        bmp_file_header file_head;
        bmp_info_header info_head;

        uint8_t *ft = reinterpret_cast<uint8_t *>(&file_head.file_type);
        ft[0] = 'B';
        ft[1] = 'M';
        file_head.reserved1 = 0;
        file_head.reserved2 = 0;
        file_head.file_size = 14 + 40 + 1024 + width * heigth * channels;
        file_head.offset_data = 54;

        out.write((char *)(&file_head), sizeof(file_head));


        info_head.size = 40;
        info_head.width = width;  // cols列1280
        info_head.height = heigth; // rows行1024
        info_head.bit_count = 8 * channels;
        info_head.planes = 1;
        info_head.compression = 0;
        info_head.size_image = width * heigth * channels;
        info_head.x_pixels_per_meter = 2835;
        info_head.y_pixels_per_meter = 2835;
        info_head.colors_used = 0;
        info_head.colors_important = 0;

        out.write((char *)(&info_head), sizeof(info_head));
        auto t1 = std::chrono::steady_clock::now();

        
        if (channels == 1)
        {
            #pragma loop(hint_parallel(12))
            for (int i = heigth - 1; i >= 0; i--)
            {
                // for (int j = 0; j < width; j++)
                
                out.write(reinterpret_cast<char *>(&imgbuffer[i * width]), sizeof(uint8_t) * heigth);
                
            }
        }
        auto t2 = std::chrono::steady_clock::now();
        auto t3 = std::chrono::system_clock::now().time_since_epoch().count() / 10000;

        out.close();
        delete [] imgbuffer;
        double dr_ms = std::chrono::duration<double,std::milli>(t2-t1).count();
        std::cout << filename << " use time: " << dr_ms  << "  " << t3 << "\n";

    }

    static void SaveVectorAsBmp(int width, int heigth, int channels,  std::string filename, std::unique_ptr<uint8_t []> p)
    {
        auto imgbuffer = p.get();

        std::ofstream out(filename, std::ios::binary);

        bmp_file_header file_head;
        bmp_info_header info_head;

        uint8_t *ft = reinterpret_cast<uint8_t *>(&file_head.file_type);
        ft[0] = 'B';
        ft[1] = 'M';
        file_head.reserved1 = 0;
        file_head.reserved2 = 0;
        file_head.file_size = 14 + 40 + 1024 + width * heigth * channels;
        file_head.offset_data = 54;

        out.write((char *)(&file_head), sizeof(file_head));


        info_head.size = 40;
        info_head.width = width;  // cols列1280
        info_head.height = heigth; // rows行1024
        info_head.bit_count = 8 * channels;
        info_head.planes = 1;
        info_head.compression = 0;
        info_head.size_image = width * heigth * channels;
        info_head.x_pixels_per_meter = 2835;
        info_head.y_pixels_per_meter = 2835;
        info_head.colors_used = 0;
        info_head.colors_important = 0;

        out.write((char *)(&info_head), sizeof(info_head));
        // auto t1 = std::chrono::steady_clock::now();

        
        if (channels == 1)
        {
            #pragma loop(hint_parallel(12))
            for (int i = heigth - 1; i >= 0; i--)
            {
                // for (int j = 0; j < width; j++)
                
                out.write(reinterpret_cast<char *>(&imgbuffer[i * width]), sizeof(uint8_t) * heigth);
                
            }
        }
        // auto t2 = std::chrono::steady_clock::now();

        out.close();
        // double dr_ms = std::chrono::duration<double,std::milli>(t2-t1).count();
        // std::cout << "use time: " << dr_ms << std::endl;

    }
};



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
    // std::ofstream fsavetime((std::string(std::string("../" + camera->cam_ip + "/") + "savetime.txt")).c_str(), std::ios::out);

    int nbuff = 0;
    size_t bufferSize = 1024 * 1280;
    int width = 1280;
    int height = 1024;

    ThreadPool pool(16);
    pool.init();
    A a;

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

    camera->camera.StartGrabbing(GrabStrategy_UpcomingImage);//GrabStrategy_UpcomingImage
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
            fs << f << " " << ff << " " << nskip << " "  << fn << "\n";
            const uint8_t* pImageBuffer = (uint8_t*) ptrGrabResult->GetBuffer();
            string filename = std::string(std::string("../" + camera->cam_ip + "/") + to_string(fn) + ".bmp");

            // std::unique_ptr<uint8_t []> p = std::make_unique<uint8_t []>(img_size);
            // std::copy(pImageBuffer, pImageBuffer + img_size, p.get());

            uint8_t *p = new uint8_t[img_size];
            std::copy(pImageBuffer, pImageBuffer + img_size, p);

            // pool.submit(A::SaveVectorAsBmpxxx, width, height, filename, p);
            A::SaveVectorAsBmpxxx(width, height, filename, p);

            // pool.submit([&width, &height, &filename, p] {
            //     A::SaveVectorAsBmpxxx(width, height, 1, filename, p);
            // });

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

    Sleep(10000);

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