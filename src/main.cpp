#include <iostream>
#include "test.hpp"
using namespace std;



int main()
{
    cout << "main start in 5s" << endl; 
    
    // cout << aurora_test() << endl;
    // cout << Grab_MultipleCameras() << endl;
    // cout << certus_test() << endl;
    GigEcameraCreateWithIp();
    return 0;
}

/*
    The CInstantCamera class uses a pool of buffers to retrieve image data
    from the camera device. Once a buffer is filled and ready,
    the buffer can be retrieved from the camera object for processing. The buffer
    and additional image data are collected in a grab result. The grab result is
    held by a smart pointer after retrieval. The buffer is automatically reused
    when explicitly released or when the smart pointer object is destroyed.

            // Only the images received last are waiting in the internal output queue
            // and are now retrieved.
            // The grabbing continues in the background, e.g. when using the hardware trigger mode.
            nBuffersInQueue = 0;
            while (camera.RetrieveResult( 0, ptrGrabResult, TimeoutHandling_Return ))
            {
                if (ptrGrabResult->GetNumberOfSkippedImages())
                {
                    cout << "Skipped " << ptrGrabResult->GetNumberOfSkippedImages() << " image." << endl;
                }
                nBuffersInQueue++;
            }

            cout << "Retrieved " << nBuffersInQueue << " grab results from output queue." << endl << endl;



            // All triggered images are still waiting in the output queue
            // and are now retrieved.
            // The grabbing continues in the background, e.g. when using hardware trigger mode,
            // as long as the grab engine does not run out of buffers.
            int nBuffersInQueue = 0;
            while (camera.RetrieveResult( 0, ptrGrabResult, TimeoutHandling_Return ))
            {
                nBuffersInQueue++;
            }
            cout << "Retrieved " << nBuffersInQueue << " grab results from output queue." << endl << endl;


            camera.MaxNumQueuedBuffer.SetToMinimum();
            Error: e1000014 The buffer was incompletely grabbed. This can be caused by performance problems of the network hardware
            used, i.e. network adapter, switch, or ethernet cable. To fix this, try increasing the camera's Inter-Packet Delay in th
            e Transport Layer category to reduce the required bandwidth, and adjust the camera's Packet Size setting to the highest
            supported frame size.

            // RetrieveResult calls the image event handler's OnImageGrabbed method.


*/


