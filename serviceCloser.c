#include <windows.h>
#include <stdio.h>
#include "service.h"

int main()
{
    // 打开服务控制管理器
    SC_HANDLE handler4ServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (handler4ServiceControlManager == NULL)
    {
        printf("OpenSCManager failed: %d\n", GetLastError());
        return 1;
    }

    // 打开服务
    SC_HANDLE hService = OpenService(handler4ServiceControlManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (hService == NULL)
    {
        printf("OpenService failed: %d\n", GetLastError());
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    // 卸载服务
    if (!DeleteService(hService))
    {
        printf("DeleteService failed: %d\n", GetLastError());
        CloseServiceHandle(hService);
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    printf("Service uninstalled successfully\n");

    CloseServiceHandle(hService);
    CloseServiceHandle(handler4ServiceControlManager);

    return 0;
}
