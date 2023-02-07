#include <iostream>
#include "test.hpp"
using namespace std;



int main()
{
    cout << "main start" << endl; 
    // cout << aurora_test() << endl;
    // cout << Grab_MultipleCameras() << endl;
    // cout << certus_test() << endl;
    GigEcameraCreateWithIp();
    return 0;
}



