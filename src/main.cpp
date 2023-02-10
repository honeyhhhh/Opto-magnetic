#include <iostream>
#include "test.hpp"
#include "camera.hpp"
#include <thread>





int main()
{
    cout << "main start" << endl; 
    PylonInitialize();


    std::thread c1(Grab_sync, CamIps[0]);
    std::thread c2(Grab_sync, CamIps[1]);
    std::thread c3(Grab_sync, CamIps[2]);
    std::thread c4(Grab_sync, CamIps[3]);

    c1.join();
    c2.join();
    c3.join();
    c4.join();



    PylonTerminate();


    return 0;
}



