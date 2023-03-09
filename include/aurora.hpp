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

bool checkDSR = false;
ndicapi* device(nullptr);
const char* name(nullptr);
const int MAX_SERIAL_PORTS = 16;
const char* reply(nullptr)





void a_get_frame();
int aurora_init();
void aurora_stop();


#endif	// _AURORA_H