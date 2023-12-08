#include <windows.h>
#include <conio.h>

void ManagerRun(LPCTSTR exeName, LPCTSTR param)
{
    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = "runas";
    ShExecInfo.lpFile = exeName;
    ShExecInfo.lpParameters = param;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    CloseHandle(ShExecInfo.hProcess);
    return;
}

int main()
{
    SC_HANDLE handler4ServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

    if (handler4ServiceControlManager == NULL)
    {
        return 1;
    }

    SC_HANDLE handler4Service = CreateService(
        handler4ServiceControlManager,  // 服务控制管理器句柄
        "OpenInCurrentScreenService",   // 服务名称
        "Open In Current Screen Service",  // 服务显示名称
        SERVICE_ALL_ACCESS,             // 服务访问权限，所有权限
        SERVICE_WIN32_OWN_PROCESS,      // 服务类型，自己的进程
        SERVICE_AUTO_START,             // 服务启动类型，自动启动
        SERVICE_ERROR_NORMAL,           // 服务错误控制类型，正常错误处理
        ".\\registerService.exe",       // 服务二进制路径
        NULL,                           // 服务加载顺序组
        NULL,                           // 服务标签标识符
        NULL,                           // 服务依赖项
        NULL,                           // 服务账户名称
        NULL                            // 服务账户密码
    );

    if (handler4Service == NULL)
    {
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    StartService(handler4Service, 0, NULL);

    CloseServiceHandle(handler4Service);
    CloseServiceHandle(handler4ServiceControlManager);

    return 0;
}