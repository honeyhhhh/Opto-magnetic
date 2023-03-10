#include <iostream>
#include <thread>
#include <chrono>

#include <windows.h>
#include <process.h>
#include "optotrak.hpp"
#include "aurora.hpp"
#include "camera.hpp"
#include "sync.hpp"
#include "serial.hpp"


#define THREAD_NUM 4
#define CAM_NUM 4

SYSTEM_INFO info;
SYNCHRONIZATION_BARRIER sb;
SYNCHRONIZATION_BARRIER sb_cam;

HANDLE hTimer = NULL;
HANDLE hTimerQueue = NULL;
HANDLE gDoneEvent;

// 需要同步的线程函数
// _beginthreadex_proc_type functions[] = {a_get_frame, c_get_frame ,cam_get_frame1, cam_get_frame1, cam_get_frame1, cam_get_frame1};
_beginthreadex_proc_type functions[] = {cam_get_frame1, cam_get_frame1, cam_get_frame1, cam_get_frame1};

// 线程标识符
HANDLE hThread[THREAD_NUM];
unsigned threadID[THREAD_NUM];

MyCamera cam[CAM_NUM];




VOID CALLBACK TimerRoutine(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    std::cout << "end timer at " << std::chrono::system_clock::now().time_since_epoch().count() / 10000 << std::endl;
    SetEvent(gDoneEvent);
}

int main()
{
    ::GetSystemInfo(&info);
    cout << "Number of processors: " << info.dwNumberOfProcessors << endl;

    // search_port();
    // COM9 cam
    // COM10 opt
    // COM5 aurora


    PylonInitialize();

    // 设置主线程优先级
    // if(!::SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS))
    // {
    //     cout << "Error 0" << endl;
    //     return 1;
    // }
    ::SetThreadAffinityMask(::GetCurrentThread(), 0x01);
    ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    // 初始化屏障
    ::InitializeSynchronizationBarrier(&sb, THREAD_NUM + 1, -1);
    ::InitializeSynchronizationBarrier(&sb_cam, CAM_NUM, -1);


    // 创建事件
    gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == gDoneEvent)
    {
        printf("CreateEvent failed (%d)\n", GetLastError());
        return 1;
    }

    // Create the timer queue.
    hTimerQueue = ::CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        printf("CreateTimerQueue failed (%d)\n", GetLastError());
        return 1;
    }

    if (THREAD_NUM == 6)
    {
        if (! certus_init())
        {
            std::cout << "certus init done" << std::endl;
        }
        else
        {
            std::cout << "certus init failed" << std::endl;

        }
        if (! aurora_init())
        {
            std::cout << "aurora init done" << std::endl;
        }
        else
        {
            std::cout << "aurora init failed" << std::endl;
            return 1;
        }
    }

    
    for (int i = 0; i < CAM_NUM; i++)
    {
        cam[i].Init(CamIps[i]);
    }
    // 参数列表
    // MyCamera *params[] = {nullptr, nullptr, &cam[1], &cam[2], &cam[3], &cam[0]};
    MyCamera *params[] = {&cam[1], &cam[2], &cam[3], &cam[0]};

    

    std::cout << "set thread" << std::endl;
    uint8_t mask =  1;
    int i = 0;
    for (auto f : functions) 
    {
        hThread[i] = (HANDLE)_beginthreadex(NULL, 0, f, params[i], CREATE_SUSPENDED, &threadID[i]); // 设置挂起
        
        // 设置 绑定核心、线程优先级
        if(!::SetThreadAffinityMask(hThread[i], mask << i))
        {
            cout << "Error 1" << endl;
            return 1;
        }

        if(!::SetThreadPriority(hThread[i], THREAD_PRIORITY_ABOVE_NORMAL))
        {
            cout << "Error 2" << endl;
            return 1;
        }
        i++;
    }

    // 唤醒
    std::cout << "wakeup thread" << std::endl;

    for (auto t : hThread)
    {
        ::ResumeThread(t);
    }


    PVOID p = &sb;
    auto barrier = (PSYNCHRONIZATION_BARRIER)p;
    ::EnterSynchronizationBarrier(barrier, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);
    // 相机硬件触发
    send_code("COM9", 4);
    send_code("COM9", 3);

    std::cout << "start timer at " << std::chrono::system_clock::now().time_since_epoch().count() / 10000 << std::endl;
    // Set a timer to call the timer routine in 10 seconds.
    if (!::CreateTimerQueueTimer( &hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, NULL , 5000, 0, 0))
    {
        printf("CreateTimerQueueTimer failed (%d)\n", GetLastError());
        return 1;
    }


    std::cout << "wait thread" << std::endl;
    std::cout << ::WaitForMultipleObjects(4, hThread, TRUE, INFINITE) << std::endl;

    std::cout << "shutdown system !" << std::endl;

    // 关闭
    for (auto t : hThread)
    {
        ::CloseHandle(t);
    }
    if (::WaitForSingleObject(gDoneEvent, INFINITE) != WAIT_OBJECT_0)
        printf("WaitForSingleObject failed (%d)\n", GetLastError());

    ::CloseHandle(gDoneEvent);

    // Delete all timers in the timer queue.
    if (!::DeleteTimerQueue(hTimerQueue))
        printf("DeleteTimerQueue failed (%d)\n", GetLastError());

    if (THREAD_NUM == 6)
    {
        certus_stop();
        aurora_stop();
    }

    PylonTerminate();
    return 0;
}



