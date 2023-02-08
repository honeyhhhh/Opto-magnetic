#ifndef TEST_H
#define TEST_H
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>
#include <pylon/CameraEventHandler.h>
#ifdef PYLON_WIN_BUILD
    #include <pylon/PylonGUI.h>
#endif
#include "ndicapi.h"
extern "C"
{

#include "ndtypes.h"
#include "ndpack.h"
#include "ndopto.h"

#include "certus_aux.h"
// #include "ot_aux.h"
}

using namespace std;
using namespace Pylon;
using namespace Basler_UniversalCameraParams;





#define SAMPLE_MARKERFREQ	2500.0f
#define SAMPLE_FRAMEFREQ	200.0f
#define SAMPLE_DUTYCYCLE	0.35f
#define SAMPLE_VOLTAGE		7.0f
#define SAMPLE_STREAMDATA	0

#define CERTUS_SAMPLE_STROBER_MARKERSTOFIRE		2
#define CERTUS_SAMPLE_3020_STROBER_MARKERSTOFIRE	6


static const uint32_t c_countOfImagesToGrab = 10;
static const size_t c_maxCamerasToUse = 4;

struct ndicapi;


// 非同步 采集图像
int Grab_MultipleCameras();



void GigEcameraCreateWithIp();



int certus_test();

int aurora_test();



#endif	// TEST