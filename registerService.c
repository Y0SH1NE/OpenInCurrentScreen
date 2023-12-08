// 创建Windows服务涉及到Windows API的使用，以下是一个基本的步骤：

// 创建一个函数来处理服务的命令。这个函数需要符合LPHANDLER_FUNCTION_EX类型的要求。

// 在main函数中，使用RegisterServiceCtrlHandlerEx函数来注册你的服务处理函数。

// 创建一个无限循环来模拟服务的运行。在这个循环中，你可以检查服务的状态，并根据需要执行相应的操作。

// 使用CreateService函数来创建服务。你需要提供服务的名称，显示名称，访问权限，服务类型，启动类型，错误控制类型，二进制路径名，加载顺序组，标记，依赖关系，服务启动账户和密码。

// 使用StartServiceCtrlDispatcher函数来连接服务控制程序。

// 以下是一个简单的示例：

#include <windows.h>
#include <stdio.h>

SERVICE_STATUS_HANDLE serviceStatusHandler;
SERVICE_STATUS serviceStatus;
HHOOK hHook; // 全局变量，用于在ServiceMain和ServiceHandlerEx之间共享钩子句柄

void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
DWORD WINAPI ServiceHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
LRESULT CALLBACK HookProc4WhCbt(int nCode, WPARAM wParam, LPARAM lParam);

int main()
{
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        { "OpenInCurrentScreenService", ServiceMain },
        { NULL, NULL }
    };

    // 启动服务的控制分发器，连接服务控制程序
    if (!StartServiceCtrlDispatcher(ServiceTable))
    {
        // 处理错误
    }

    return 0;
}

// 服务的主函数：这是服务的入口点，当服务控制管理器启动服务时，会调用这个函数。这个函数需要符合LPSERVICE_MAIN_FUNCTION类型的要求。
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    //注册服务的控制处理程序
    serviceStatusHandler = RegisterServiceCtrlHandlerEx("OpenInCurrentScreenService", ServiceHandlerEx, NULL);

    ZeroMemory(&serviceStatus, sizeof(serviceStatus));
    serviceStatus.dwServiceType = SERVICE_WIN32;
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    SetServiceStatus(serviceStatusHandler, &serviceStatus);

    // 安装钩子
    hHook = SetWindowsHookEx(WH_CBT, HookProc4WhCbt, NULL, 0);
    if (hHook == NULL)
    {
        return;
    }

    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(serviceStatusHandler, &serviceStatus);

    while (1)
    {
        Sleep(100000);
    }
}

// 服务的控制处理程序：用于处理来自服务控制管理器的控制请求，如停止、暂停、继续和关机等。这个函数需要符合LPHANDLER_FUNCTION_EX类型的要求。
DWORD WINAPI ServiceHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    switch (dwControl)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(serviceStatusHandler, &serviceStatus);

            // 卸载钩子
            UnhookWindowsHookEx(hHook);

            serviceStatus.dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(serviceStatusHandler, &serviceStatus);
            return NO_ERROR;
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

LRESULT CALLBACK HookProc4WhCbt(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HCBT_CREATEWND)
    {
        /*
        * 此时wParam为被创建的窗口的句柄，
        * lParam为指向CBT_CREATEWND结构的指针
        * CBT_CREATEWND结构定义如下：
        * typedef struct tagCBT_CREATEWNDA
        * {
        *     struct tagCREATESTRUCTA *lpcs; 
        *     HWND hwndInsertAfter;          //窗口句柄，表示新窗口将被插入到哪个窗口之后。
        * } CBT_CREATEWNDA, *LPCBT_CREATEWNDA;
        */
        HWND hWnd = (HWND)wParam;
        HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &monitorInfo);

        // 获取鼠标当前所在屏幕的工作区域
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        HMONITOR hMonitorCursor = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfoCursor;
        monitorInfoCursor.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitorCursor, &monitorInfoCursor);

        // 判断触发事件的窗口是否在当前屏幕
        if (hMonitor != hMonitorCursor)
        {
            // 获取窗口位置
            RECT windowRect;
            GetWindowRect(hWnd, &windowRect);

            // 计算窗口在当前屏幕的位置
            int offsetX = monitorInfoCursor.rcWork.left - monitorInfo.rcWork.left;
            int offsetY = monitorInfoCursor.rcWork.top - monitorInfo.rcWork.top;
            int newLeft = windowRect.left + offsetX;
            int newTop = windowRect.top + offsetY;

            // 移动窗口到当前屏幕
            SetWindowPos(hWnd, NULL, newLeft, newTop, SWP_NOSIZE, SWP_NOSIZE, SWP_NOSIZE | SWP_NOZORDER);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
