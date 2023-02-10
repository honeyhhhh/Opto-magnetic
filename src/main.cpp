#include <iostream>
#include "test.hpp"
#include "camera.hpp"
#include <thread>
#include <chrono>
#include <mutex>




int main()
{
    cout << "main start" << endl; 
    cout << "CPU num: " << std::thread::hardware_concurrency() << endl;
    PylonInitialize();

    std::thread c[4];

    for (int i = 0; i < 4; i++)
    {
        c[i] = std::thread(Grab_sync, CamIps[i]);
    }

    for (int i = 0; i < 4; i++)
    {
        c[i].detach();
    }
    cout << "hello" << endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    cam_ready = true;



    std::this_thread::sleep_for(std::chrono::seconds(5));
    PylonTerminate();


    return 0;
}



