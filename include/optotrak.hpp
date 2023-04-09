#ifndef _OPTOTRAK_H 
#define _OPTOTRAK_H 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <fstream>
extern "C"
{

#include "ndtypes.h"
#include "ndpack.h"
#include "ndopto.h"

#include "certus_aux.h"
// #include "ot_aux.h"
}
#include "sync.hpp"



#define SAMPLE_MARKERFREQ	2500.0f
#define SAMPLE_FRAMEFREQ	200.0f
#define SAMPLE_DUTYCYCLE	0.35f
#define SAMPLE_VOLTAGE		7.0f
#define SAMPLE_STREAMDATA	0

#define CERTUS_SAMPLE_STROBER_MARKERSTOFIRE		2
#define CERTUS_SAMPLE_3020_STROBER_MARKERSTOFIRE	6

extern OptotrakSettings dtSettings;
extern char szNDErrorString[MAX_ERROR_STRING_LENGTH + 1];
extern char szProperty[32];


int certus_init();
int certus_init4();

unsigned __stdcall c_get_frame(LPVOID);
void certus_stop();
int certus_error_exit();

void c_get_frame2();

void c_get_frame4();

















#endif	// _OPTOTRAK_H