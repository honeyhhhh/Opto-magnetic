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



// 需要同步的线程函数
LPTHREAD_START_ROUTINE functions[] = {c_get_frame, a_get_frame, cam_get_frame, cam_get_frame, cam_get_frame, cam_get_frame};

// 线程标识符
HANDLE hThread[6];
unsigned threadID[6];

MyCamera cam[4];



VOID CALLBACK TimerRoutine(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    SetEvent(gDoneEvent);
}

int main()
{
    GetSystemInfo(&info);
    cout << "Number of processors: " << info.dwNumberOfProcessors << endl;
    
    PylonInitialize();

    // 设置主线程优先级
    if(!SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS))
    {
        cout << "Error 0" << endl;
        return 1;
    }

    // 初始化屏障
    InitializeSynchronizationBarrier(&sb, 6, -1);

    // 创建事件
    gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == gDoneEvent)
    {
        printf("CreateEvent failed (%d)\n", GetLastError());
        return 1;
    }

    // Create the timer queue.
    hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        printf("CreateTimerQueue failed (%d)\n", GetLastError());
        return 1;
    }


    certus_init();
    aurora_init();


    for (int i = 0; i < 4; i++)
    {
        cam[i].Init(CamIps[i]);
    }
    // 参数列表
    MyCamera *params[] = {nullptr, nullptr, &cam[1], &cam[2], &cam[3], &cam[0]}


    int i = 0;
    for (auto f : functions) 
    {
        hThread[i] = (HANDLE)_beginthreadex(NULL, 0, f, params[i], CREATE_SUSPENDED, &threadID[i]); // 设置挂起

        // 设置 绑定核心、线程优先级
        if(!SetThreadAffinityMask(hThread[i], 1 << i))
        {
            cout << "Error 1" << endl;
            return 1;
        }

        if(!SetThreadPriority(hThread[i], THREAD_PRIORITY_ABOVE_NORMAL))
        {
            cout << "Error 2" << endl;
            return 1;
        }
        i++;
    }
    // 相机硬件触发
    search_port();

    // 唤醒
    for (auto t : hThread)
    {
        ResumeThread(t);
    }

    // Set a timer to call the timer routine in 10 seconds.
    if (!::CreateTimerQueueTimer( &hTimer, hTimerQueue, (WAITORTIMERCALLBACK)TimerRoutine, NULL , 10000, 0, 0))
    {
        printf("CreateTimerQueueTimer failed (%d)\n", GetLastError());
        return 1;
    }


    
    ::WaitForMultipleObjects(_countof(hThread), hThread, TRUE, INFINITE);


    // 关闭
    for (auto t : hThread)
    {
        CloseHandle(t);
    }
    if (::WaitForSingleObject(gDoneEvent, INFINITE) != WAIT_OBJECT_0)
        printf("WaitForSingleObject failed (%d)\n", GetLastError());

    CloseHandle(gDoneEvent);

    // Delete all timers in the timer queue.
    if (!::DeleteTimerQueue(hTimerQueue))
        printf("DeleteTimerQueue failed (%d)\n", GetLastError());

    certus_stop();
    aurora_stop();

    PylonTerminate();
    return 0;
}



