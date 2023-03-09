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



extern SYSTEM_INFO info;
extern SYNCHRONIZATION_BARRIER sb;

extern HANDLE hTimer;
extern HANDLE hTimerQueue;
extern HANDLE gDoneEvent;



#endif	// _SYNC_H