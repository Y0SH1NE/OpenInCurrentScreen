#include <windows.h>
#include "service.h"
#include <string.h>

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

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        ManagerRun(argv[0], "2");
        return 1;
    }
    SC_HANDLE handler4ServiceControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

    if (handler4ServiceControlManager == NULL)
    {
        return 1;
    }

    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    char* lastSlash = strrchr(path, '\\');
    if (lastSlash != NULL) {
        strcpy(lastSlash + 1, SERVICE_NAME".exe");
    }
    SC_HANDLE handler4Service = CreateService(
        handler4ServiceControlManager,  // 服务控制管理器句柄
        SERVICE_NAME,                   // 服务名称
        SERVICE_NAME,                   // 服务显示名称
        SERVICE_ALL_ACCESS,             // 服务访问权限，所有权限
        SERVICE_WIN32_OWN_PROCESS,      // 服务类型，自己的进程
        SERVICE_AUTO_START,             // 服务启动类型，自动启动
        SERVICE_ERROR_NORMAL,           // 服务错误控制类型，正常错误处理
        path,                           // 服务程序二进制路径
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

    // 设置服务描述
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = "Make the window always open in the screen where the mouse is located\n"
        "\xe4\xbd\xbf\xe7\xaa\x97\xe5\x8f\xa3\xe6\x80\xbb\xe6\x98\xaf\xe6\x89\x93\xe5\xbc\x80\xe5\x9c\xa8\xe9\xbc\xa0\xe6\xa0\x87\xe6\x89\x80\xe5\x9c\xa8\xe7\x9a\x84\xe5\xb1\x8f\xe5\xb9\x95\xe4\xb8\x8a\n"
        "\xe4\xbd\xbf\xe7\xaa\x97\xe5\x8f\xa3\xe7\xb8\xbd\xe6\x98\xaf\xe6\x89\x93\xe9\x96\x8b\xe5\x9c\xa8\xe6\xbb\x91\xe9\xbc\xa0\xe6\x89\x80\xe5\x9c\xa8\xe7\x9a\x84\xe8\x9e\xa2\xe5\xb9\x95\xe4\xb8\x8a\n"
        "\xe3\x83\x9e\xe3\x82\xa6\xe3\x82\xb9\xe3\x81\x8c\xe3\x81\x82\xe3\x82\x8b\xe7\x94\xbb\xe9\x9d\xa2\xe3\x81\xa7\xe3\x82\xa6\xe3\x82\xa3\xe3\x83\xb3\xe3\x83\x89\xe3\x82\xa6\xe3\x82\x92\xe9\x96\x8b"
        "\xe3\x81\x8f\xe3\x82\x88\xe3\x81\x86\xe3\x81\xab\xe3\x81\x99\xe3\x82\x8b\n";
// 使窗口总是打开在鼠标所在的屏幕上\n使窗口總是打開在滑鼠所在的螢幕上\nマウスがある画面でウィンドウを開くようにする\n"

    if (!ChangeServiceConfig2(handler4Service, SERVICE_CONFIG_DESCRIPTION, &sd)) {
        CloseServiceHandle(handler4Service);
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    StartService(handler4Service, 0, NULL);

    CloseServiceHandle(handler4Service);
    CloseServiceHandle(handler4ServiceControlManager);

    return 0;
}
