#include "camera.hpp"


inline int bmpEncoder(const std::string &location,
    const unsigned __int32 &width, const unsigned __int32 &height,
    const std::vector<BYTE> &buffer,
    const bool &hasAlphaChannel = false) {

    
    std::ofstream fout(location, std::ios::out | std::ios::binary);

    if (fout.fail()) {
        return 0;
    }

    //Padding
    const unsigned __int8 padding = hasAlphaChannel ? 0 : (4 - (width * 3) % 4) % 4;

    //Bitmap file header.
    const char signature[2] = { 'B', 'M' };
    const unsigned __int32 fileSize = buffer.size() * sizeof(BYTE) + padding * (height - 1) + 14 + 124;
    const unsigned __int32 offset = 14 + 124;

    //Bitmap information header file
    const unsigned __int32 DIBSize = 124;
    const __int32 bitmapWidth = width;
    const __int32 bitmapHeight = height;
    const unsigned __int16 numPlanes = 1;
    const unsigned __int16 bitsPerPixel = (hasAlphaChannel) ? 32 : 24;
    const unsigned __int32 compressionMethod = (hasAlphaChannel) ? 3 : 0; //BI_RGB = 0, BI_BITFIELDS = 3
    const unsigned __int32 bitmapSize = buffer.size() * sizeof(BYTE);
    const __int32 horizontalResolution = 2834;
    const __int32 verticalResolution = 2834;
    const unsigned __int32 numColors = 0;
    const unsigned __int32 impColorCount = 0;
    const unsigned __int32 redBitmask = (hasAlphaChannel) ? 0x0000FF00 : 0; //ARGB32 pixel format
    const unsigned __int32 greenBitmask = (hasAlphaChannel) ? 0x00FF0000 : 0;
    const unsigned __int32 blueBitmask = (hasAlphaChannel) ? 0xFF000000 : 0;
    const unsigned __int32 alphaBitmask = (hasAlphaChannel) ? 0x000000FF : 0;

    //Writing the file header and information header to the file 
    std::vector<BYTE> header(offset, 0);
    header[0] = signature[0];
    header[1] = signature[1];

#define HEADERS(i, variableName)    header[i] = variableName; header[i+1] = variableName >> 8; header[i+2] = variableName >> 16; header[i+3] = variableName >> 24;

    HEADERS(2, fileSize);
    HEADERS(6, 0);
    HEADERS(10, offset);
    HEADERS(14, DIBSize);
    HEADERS(18, bitmapWidth);
    HEADERS(22, bitmapHeight);

    header[26] = (BYTE)numPlanes;
    header[27] = (BYTE)(numPlanes >> 8);
    header[28] = (BYTE)bitsPerPixel;
    header[29] = (BYTE)(bitsPerPixel >> 8);

    HEADERS(30, compressionMethod);
    HEADERS(34, bitmapSize);
    HEADERS(38, horizontalResolution);
    HEADERS(42, verticalResolution);
    HEADERS(46, numColors);
    HEADERS(50, impColorCount);
    HEADERS(54, redBitmask);
    HEADERS(58, greenBitmask);
    HEADERS(62, blueBitmask);
    HEADERS(66, alphaBitmask);

#undef HEADERS

    fout.write((char *)header.data(), sizeof(BYTE) * header.size());

    //Writing the pixel array
    const unsigned __int32 bWidth = bitsPerPixel / 8 * width;

    for (int i = height - 1; i >= 0; i--) {
        std::vector<BYTE> row(buffer.begin() + i * bWidth, buffer.begin() + i * bWidth + bWidth);
        fout.write((char *)row.data(), row.size() * sizeof(BYTE));
        fout.seekp(padding * sizeof(BYTE), std::ios::cur);
    }

    fout.close();
    return 1;
}


MyCamera::MyCamera()
{

}

void MyCamera::Init(string camip, bool evenon = false)
{
    cam_ip = camip;
    event_on = evenon;

    auto t0 = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
    cout << "init camera :" << cam_ip << " at " << t0 << endl; // cameras ticks same ? set min?

    CTlFactory& TlFactory = CTlFactory::GetInstance();
    CDeviceInfo di;
    di.SetIpAddress(cam_ip.c_str());

    camera.Attach(TlFactory.CreateDevice(di));



    camera.Open();


    camera.GevTimestampControlReset();
    auto t1 = std::chrono::system_clock::now().time_since_epoch().count();
    camera.GevTimestampControlLatch.Execute();
    auto t2 = std::chrono::system_clock::now().time_since_epoch().count();
    auto tt = camera.GevTimestampValue.GetValue() / 1000000;

    auto t3 = (t1 + t2) / 2 / 10000; 
    time_differ = t3 - tt;
    // cout << "camera :" << camip << " at " << t1/10000 << " " << t2/10000 << " in " << tt << endl;




    camera.ExposureTimeAbs.TrySetValue(15000.0);
    camera.AcquisitionFrameRateEnable.SetValue(true);
    camera.AcquisitionFrameRateAbs.SetValue(40.0);
    camera.Width.SetValue(1280);
    camera.Height.SetValue(1024);
    // camera.TriggerMode.SetValue(TriggerMode_Off);

    camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
    camera.TriggerMode.SetValue(TriggerMode_On);
    camera.TriggerSource.SetValue(TriggerSource_Line1);

    if (event_on)
    {
        CSampleCameraEventHandler* pHandler1 = new CSampleCameraEventHandler;
        camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete );
        camera.GrabCameraEvents = true;

        camera.RegisterCameraEventHandler( pHandler1, "ExposureEndEventData", eMyExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_None );
        camera.RegisterCameraEventHandler( pHandler1, "FrameStartEventTimestamp", eMyFrameStartEvent, RegistrationMode_Append, Cleanup_None );
        camera.RegisterCameraEventHandler( pHandler1, "AcquisitionStartEventTimestamp", eMyAcquisitionStartEvent, RegistrationMode_Append, Cleanup_None );

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
    }

    camera.MaxNumBuffer = 5;
    camera.OutputQueueSize = 5;


    camera.StartGrabbing(GrabStrategy_UpcomingImage);
    auto t4 = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
    std::cout << camip << " initdone at " << t4  << std::endl;
}


MyCamera::~MyCamera()
{
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
    return t + this.time_differ;
}





void cam_get_frame(MyCamera *camera)
{
    std::ofstream fs((std::string(std::string("../" + camera->cam_ip + "/") + "time.txt")).c_str(), std::ios::out);
    std::ofstream fn((std::string(std::string("../" + camera->cam_ip + "/") + "num.txt")).c_str(), std::ios::out);

    int nbuff = 0;
    size_t bufferSize = 1024 * 1280;

    CBaslerUniversalGrabResultPtr ptrGrabResult;
    std::vector<uint64_t> cam_Time(500);
    std::vector<unsigned long> frame_Number(500);
    std::vector<std::vector<uint8_t> > cam_Data (500, std::vector<uint8_t>(bufferSize));


    // 进入屏障
    ::EnterSynchronizationBarrier(sb, SYNCHRONIZATION_BARRIER_FLAGS_SPIN_ONLY);

    auto t1 = std::chrono::system_clock::now();

    while (camera->RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_Return ))
    {
        // auto t5 = std::chrono::system_clock::now().time_since_epoch().count();
        // cout << "camera : " << camera.GetDeviceInfo().GetIpAddress() << " ";
        // cout << "ImageNumber: " << ptrGrabResult->GetImageNumber() << "\tExposure start ts: " << Time_convert(ptrGrabResult->GetTimeStamp()) + 50 << " " << t5/10000 << " " << ptrGrabResult->GetNumberOfSkippedImages() << endl;

        if (ptrGrabResult->GrabSucceeded())
        {
            uint64_t f = camera->Time_convert(ptrGrabResult->GetTimeStamp()); //exposure start
            const uint8_t* pImageBuffer = (uint8_t*) ptrGrabResult->GetBuffer();
            std::vector<uint8_t> t(pImageBuffer, pImageBuffer + bufferSize);

            cam_Time.push_back(f);
            cam_Data.push_back(t);
        }

		if (WaitForSingleObject(gDoneEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}
    }

    auto t2 = std::chrono::steady_clock::now();
    // save
	std::cout << "size" << cam_Time.size() << "\t" << cam_Data.size() << std::endl;
    for (auto t : cam_Time)
		fs << t << "\n";

    int i = 0;
    for (auto t : data)
    {
        bmpEncoder(std::string(std::string("../" + camera->cam_ip + "/") + to_string(i) + ".bmp"), 1280, 1024, t, false);
        i++;
    }


    auto t3 = std::chrono::steady_clock::now();

    std::cout << "cam_frame_cout " << 0 << "\t" << "times :" << t1.time_since_epoch().count() / 10000 << "~" << t2.time_since_epoch().count() / 10000<< "~" << t3.time_since_epoch().count() / 10000 << std::endl;

    fs.close();
    fn.close();


    camera->StopGrabbing();
    _endthreadex(0);
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