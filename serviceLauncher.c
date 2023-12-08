#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "service.h"
#include "managerRun.h"

char* GetServiceDescription()
{
    // char* description;
    // FILE *fp = fopen("serviceDescription.txt", "r");
    // if (fp == NULL)
    // {
    //     return NULL;
    // }
    // fseek(fp, 0, SEEK_END);
    // long size = ftell(fp);
    // fseek(fp, 0, SEEK_SET);
    // description = (char*)malloc(size + 1);
    // fread(description, 1, size, fp);
    // description[size] = '\0';
    // fclose(fp);

    // return description;
    return  "Make the window always open in the screen where the mouse is located\n";
            // "使窗口总是打开在鼠标所在的屏幕上\n"
            // "使窗口總是打開在滑鼠所在的螢幕上\n"
            // "マウスがある画面でウィンドウを開くようにする";
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
        MessageBox(NULL, "failed to OpenSCManager()", "tip", MB_OK);
        return 1;
    }

    // 检查服务是否已经存在
    SC_HANDLE handler4Service = OpenService(handler4ServiceControlManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (handler4Service != NULL)
    {
        MessageBox(NULL, "Service already exsit", "tip", MB_OK);
        CloseServiceHandle(handler4Service);
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    char* lastSlash = strrchr(path, '\\');
    if (lastSlash != NULL) {
        strcpy(lastSlash + 1, SERVICE_NAME".exe");
    }
    handler4Service = CreateService(
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
        MessageBox(NULL, "failed to CreateService()", "tip", MB_OK);
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    // 设置服务描述
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = GetServiceDescription();


    if (!ChangeServiceConfig2(handler4Service, SERVICE_CONFIG_DESCRIPTION, &sd)) {
        MessageBox(NULL, "failed to set description", "tip", MB_OK);
        CloseServiceHandle(handler4Service);
        CloseServiceHandle(handler4ServiceControlManager);
        return 1;
    }

    StartService(handler4Service, 0, NULL);

    CloseServiceHandle(handler4Service);
    CloseServiceHandle(handler4ServiceControlManager);

    MessageBox(NULL, "success to Run Service", "tip", MB_OK);

    return 0;
}
