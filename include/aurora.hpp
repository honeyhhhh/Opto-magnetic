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
#include "sync.hpp"
#include <vector>

extern bool checkDSR;
extern ndicapi* device;
extern const char* name;
extern const int MAX_SERIAL_PORTS;
extern const char* reply;





unsigned __stdcall a_get_frame(LPVOID);
int aurora_init();
void aurora_stop();
void a_get_frame2();

#endif	// _AURORA_H