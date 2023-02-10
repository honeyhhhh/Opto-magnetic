#ifndef _SYNC_H 
#define _SYNC_H 
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>


std::mutex cam_mutex;
std::condition_variable cam_cond;
int cam_latch = 4;
bool cam_ready = false;



#endif	// _SYNC_H