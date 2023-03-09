#ifndef _SYNC_H 
#define _SYNC_H 
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <fstream>
#include <windows.h>
#include <process.h>

std::mutex cam_mutex;
std::condition_variable cam_cond;
int cam_latch = 4;
bool cam_ready = false;


SYSTEM_INFO info;
SYNCHRONIZATION_BARRIER sb;

HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;
HANDLE gDoneEvent;



#endif	// _SYNC_H