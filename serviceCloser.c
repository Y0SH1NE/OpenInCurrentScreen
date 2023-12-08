#include <windows.h>
#include <stdio.h>
#include "service.h"
#include "managerRun.h"


int main(int argc, char *argv[])
{
    
    if(argc < 2)
    {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        ManagerRun(argv[0], "2");
        return 1;
    }
    // 打开服务控制管理器
    SC_HANDLE handler4ServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (handler4ServiceControlManager == NULL)
    {
        MessageBox(NULL, "failed to OpenSCManager()", "tip", MB_OK);
        printf("OpenSCManager failed: %d\n", GetLastError());
        return 1;
    }

    // 打开服务
    SC_HANDLE hService = OpenService(handler4ServiceControlManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (hService == NULL)
    {
        MessageBox(NULL, "Service not exsit", "tip", MB_OK);
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    // 停止服务
    SERVICE_STATUS status;
    if(!ControlService(hService, SERVICE_CONTROL_STOP, &status))
    {
        MessageBox(NULL, "fail to Stop Service", "tip", MB_OK);
        CloseServiceHandle(hService);
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    // 卸载服务
    if (!DeleteService(hService))
    {
        MessageBox(NULL, "fail to Delete Service", "tip", MB_OK);
        CloseServiceHandle(hService);
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    printf("Service uninstalled successfully\n");

    CloseServiceHandle(hService);
    CloseServiceHandle(handler4ServiceControlManager);

    MessageBox(NULL, "success", "tip", MB_OK);
    return 0;
}
