#include "aurora.hpp"


bool checkDSR = false;
ndicapi* device(nullptr);
const char* name(nullptr);
const int MAX_SERIAL_PORTS = 16;
const char* reply(nullptr);


int aurora_init()
{
    for (int i = 0; i < MAX_SERIAL_PORTS; ++i)
    {
        name = ndiSerialDeviceName(i);
        int result = ndiSerialProbe(name,checkDSR);
        if (result == NDI_OKAY)
        {
            break;
        }
    }
    if (name != nullptr)
    {
        int result = ndiSerialProbe(name,checkDSR);
        // 打开串口
        device = ndiOpenSerial(name);
    }
    else
    {
        std::cout << "not found port!" << std::endl;
        return 1;
    }

    if (device != nullptr)
    {
        reply = ndiCommand(device, "VER:4");
        //fprintf(logg, "------ system version ------ \n %s", reply);
        /*
            Init system and  Set System COM Parms
        */
        //fprintf(logg, "----- Init system and  Set System COM Parms -----\n");
        reply = ndiCommand(device, "INIT:");
        //fprintf(logg, "send INIT command :%s\n", reply);

        if (strncmp(reply, "ERROR", strlen(reply)) == 0 || ndiGetError(device) != NDI_OKAY)
        {
            std::cerr << "Error when sending command: " << ndiErrorString(ndiGetError(device)) << std::endl;
            return EXIT_FAILURE;
        }
        reply = ndiCommand(device, "COMM:%d%03d%d", NDI_115200, NDI_8N1, NDI_NOHANDSHAKE);
        //fprintf(logg, "send connect serial command :%s\n", reply);
        /*
            Initialize All Port handle
        */
        reply = ndiCommand(device, "PHSR:01");
        //fprintf(logg, "send PHSR:01 command Are there port handles to be freed?:%s\n", reply);
        reply = ndiCommand(device, "PHSR:02");
        //fprintf(logg, "send PHSR:02 command Are there port handles to be initialized?:%s\n", reply);
        int number_of_tools = ndiGetPHSRNumberOfHandles(device); // 电磁标记点
        //fprintf(logg, "found %d PHSRNumberOfHandles need init\n", number_of_tools);

        while (number_of_tools > 0)
        {
            for (int ndi_tool_index = 0; ndi_tool_index < number_of_tools; ndi_tool_index++)
            {
                int port_handle = ndiGetPHSRHandle(device, ndi_tool_index);
                //fprintf(logg, "The PHINF command can be used to get detailed information about the port handle: %02X\n",port_handle);
                reply = ndiCommand(device, "PINIT:%02X",port_handle);
                //fprintf(logg, "send PINIT:%02X command Initialize port handles:%s\n", port_handle,reply);

            }
            reply = ndiCommand(device, "PHSR:02");
            //fprintf(logg, "send PHSR:02 command Are there port handles to be initialized?:%s\n", reply);
            number_of_tools = ndiGetPHSRNumberOfHandles(device);
            //fprintf(logg, "found %d PHSRNumberOfHandles\n", number_of_tools);
        }

        /*
            Enable All Ports
        */
        reply = ndiCommand(device, "PHSR:03");
        //fprintf(logg, "send PHSR:03 command  Are there port handles to be enabled?:%s\n", reply);
        number_of_tools = ndiGetPHSRNumberOfHandles(device); // 电磁标记点
        //fprintf(logg, "found %d PHSRNumberOfHandles need activate\n", number_of_tools);

        for (int ndi_tool_index = 0; ndi_tool_index < number_of_tools; ndi_tool_index++)
        {
            int port_handle = ndiGetPHSRHandle(device, ndi_tool_index);
            //fprintf(logg, "The PHINF command can be used to get detailed information about the port handle: %02X\n",port_handle);
            reply = ndiCommand(device, "PENA:%02X%c",port_handle,'D');
            //fprintf(logg, "send PENA:%02X%c command Enable port handles:%s\n", port_handle,'D',reply);
        }
    
        /*
            Data tansform mode
            "TX Mode (ASCII return for allocated port handles)", TX:0801
            "BX Mode (Binary return for allocated port handles)", fast but aurora 008 not support
        */
        
        reply = ndiCommand(device, "TSTART:");
        if (strncmp(reply, "ERROR", strlen(reply)) == 0 || ndiGetError(device) != NDI_OKAY)
        {
            std::cerr << "Error when sending command: " << ndiErrorString(ndiGetError(device)) << std::endl;
            return EXIT_FAILURE;
        }

        return 0;
    }
    else
    {
        std::cout << "not device !" << std::endl;
        return 1;
    }
}


void aurora_stop()
{
    reply = ndiCommand(device, "TSTOP:");
    if (strncmp(reply, "ERROR", strlen(reply)) == 0 || ndiGetError(device) != NDI_OKAY)
    {
        std::cerr << "Error when sending command: " << ndiErrorString(ndiGetError(device)) << std::endl;
    }

        // 关闭串口
    ndiCloseSerial(device);
}

unsigned __stdcall a_get_frame(LPVOID)
{
    std::ofstream fs("../dataset/mag_timestamp.txt", std::ios::out);
    std::ofstream fd("../dataset/mag_data.txt", std::ios::out);
    std::ofstream fn("../dataset/mag_num.txt", std::ios::out);


    std::vector<uint64_t> mag_Time(500);
    std::vector<std::vector<double> > mag_Data (500, std::vector<double>(8));
    std::vector<unsigned long> frame_Number(500);

    // 进入屏障
	std::cout << "aurora thread into barrier" << std::endl;

    PVOID p = &sb;
    auto barrier = (PSYNCHRONIZATION_BARRIER)p;
    ::EnterSynchronizationBarrier(barrier, SYNCHRONIZATION_BARRIER_FLAGS_SPIN_ONLY);
	std::cout << "aurora thread start !" << std::endl;

    auto t1 = std::chrono::system_clock::now();
    int frame_count = 0;
    while(1)
    {
        double trans[8];
		if (WaitForSingleObject(gDoneEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}

        auto t = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
        const char* re = ndiTX(device, 0x0801);
        unsigned long frame_number =  ndiGetTXFrame(device, 10);
        int tfStatus = ndiGetTXTransform(device, 10, trans);
        frame_count++;

        fs << t << "\n";
		fd << trans[0] << " " << trans[1] << " " << trans[2] << " " << trans[3] << " " << trans[4] << " " << trans[5] << " " << trans[6] << " " << trans[7] << "\n";
        fn << frame_number << "\n";
        // mag_Time.push_back(t);
        // mag_Data.push_back(std::vector<double>(std::begin(trans), std::end(trans)));
        // frame_Number.push_back(frame_number);

    }

    auto t2 = std::chrono::system_clock::now();

    std::cout << "mag_frame_cout " << frame_count << "\t";
    std::cout << "times :" << t1.time_since_epoch().count() << "~";
    std::cout << t2.time_since_epoch().count() << std::endl;

    
    fs.close();
    fn.close();
    fd.close();

    _endthreadex(0);
    return 0;

}

void a_get_frame2()
{
    std::ofstream fs("../static_data/mag_timestamp.txt", std::ios::out | std::ios::app);
    std::ofstream fd("../static_data/mag_data.txt", std::ios::out | std::ios::app);
    std::ofstream fn("../static_data/mag_num.txt", std::ios::out | std::ios::app);


    double trans[8];

    auto t = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
    const char* re = ndiTX(device, 0x0801);
    unsigned long frame_number =  ndiGetTXFrame(device, 10);
    int tfStatus = ndiGetTXTransform(device, 10, trans);

    fs << t << "\n";
	fd << trans[0] << " " << trans[1] << " " << trans[2] << " " << trans[3] << " " << trans[4] << " " << trans[5] << " " << trans[6] << " " << trans[7] << "\n";
    fn << frame_number << "\n";

    

    std::cout << "mag_frame " << "\n";
    
    fs.close();
    fn.close();
    fd.close();


}