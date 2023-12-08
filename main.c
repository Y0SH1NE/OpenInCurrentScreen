
#include <windows.h>

/*
* @param nCode: 具体的事件类型，如HCBT_ACTIVATE、HCBT_CREATEWND等。
* @param wParam: 指定与nCode相关联的消息的附加消息信息。
* @param lParam: 指定与nCode相关联的消息的附加消息信息。
* @return: 如果nCode小于0，则钩子子程必须返回CallNextHookEx函数的返回值；
*/
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

int main()
{
    // 安装钩子
    // WH_CBT: 用于截获应用程序中的各种事件，如创建、移动、激活、销毁窗口、系统消息等
    HHOOK hHook = SetWindowsHookEx(WH_CBT, HookProc4WhCbt, NULL, 0);
    if (hHook == NULL)
    {
        return 1;
    }

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 卸载钩子
    UnhookWindowsHookEx(hHook);

    return 0;
}

