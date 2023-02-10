#ifndef _AURORA_H 
#define _AURORA_H 

#include <iostream>
#include "ndicapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <fstream>


void get_frame(ndicapi* device)
{
    std::ofstream fs("../mag_timestamp.txt", std::ios::out);
    std::ofstream fd("../mag_data.txt", std::ios::out);


    cout << "aurora ready !" << endl;
    while (!cam_ready){ std::this_thread::sleep_for(std::chrono::microseconds(50));}
    cout << "aurora start !" << endl;

    auto t1 = std::chrono::steady_clock::now();
    int frame_count = 0;
    while(1)
    {

        auto t = std::chrono::system_clock::now().time_since_epoch().count() / 10000;
        const char* re = ndiTX(device, 0x0801);

        ulong frame_number =  ndiGetTXFrame(device, 10);
        float trans[8];
        int tfStatus = ndiGetTXTransformf(device, 10, trans);
        frame_count++;
        if (tfStatus == NDI_MISSING || tfStatus == NDI_DISABLED)
        {
            //fprintf(logg, "bad frame\n");
        }

        //fprintf(logg, "frame %lu time:%llu\n%.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf\n", frame_number, d.count(), 
                                                // trans[0], trans[1], trans[2], trans[3], trans[4], trans[5], trans[6], trans[7]);
        fs << t << "\n";
		fd << trans[0] << " " << trans[1] << " " << trans[2] << " " << trans[3] << " " << trans[4] << " " << trans[5] << " " << trans[6] << " " << trans[7] << "\n";
        if (frame_count > 40)
        {
            break;
        }
    }

    auto t2 = std::chrono::steady_clock::now();
    double dr_ms = std::chrono::duration<double,std::milli>(t2-t1).count();

    //fprintf(logg, "frame_count :%d ; use time :%lf\n", frame_count, dr_ms);

}




int aurora_test()
{

    // std::cout is not use
    // logg = fopen("../log.txt", "w");
    // if (logg == NULL)
    // {
    //     std::cout << "failed create log" << endl;
    // }
    //fprintf(logg, "begin aurora test\n");

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
            //fprintf(logg, "found serial :%s\n", name);
            break;
        }
    }
    if (name != nullptr)
    {
        int result = ndiSerialProbe(name,checkDSR);
        // 打开串口
        //fprintf(logg, "open serial :%s\n", name);
        device = ndiOpenSerial(name);
    }
    else
    {
        std::cout << "not found !" << std::endl;
    }

    if (device != nullptr)
    {
        const char* reply = ndiCommand(device, "VER:4");
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
        //fprintf(logg, "----- Initialize All Port handle -----\n");
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
        //fprintf(logg, "----- Enable All Ports -----\n");

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
        


        /*
            Start tracking
        */

        reply = ndiCommand(device, "TSTART:");
        //fprintf(logg, "----- Start tracking -----\n");
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
        //fprintf(logg, "----- Stop tracking -----\n");
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

    //fprintf(logg, "end\n");

    cout << "aurora end" << endl;

    // fclose(logg);
    return 0;
}


#endif	// _AURORA_H