#include <iostream>

#include "camera.hpp"
#include <thread>
#include <chrono>
#include <mutex>
#include "optotrak.hpp"
#include "aurora.hpp"




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
    std::thread t1(certus_test);
    std::thread t2(aurora_test);
    t1.detach();
    t2.detach();
    for (int i = 0; i < 4; i++)
    {
        c[i].detach();
    }

    cout << "hello" << endl;

    std::this_thread::sleep_for(std::chrono::seconds(30));
    cam_ready = true;



    std::this_thread::sleep_for(std::chrono::seconds(20));
    PylonTerminate();


    return 0;
}



