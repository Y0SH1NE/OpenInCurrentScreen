#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "SVC_ERROR.h"
#include "service.h"
#include <string.h>

#pragma comment(lib, "advapi32.lib")

SERVICE_STATUS gSvcStatus;
SERVICE_STATUS_HANDLE gSvcStatusHandle;
HANDLE ghSvcStopEvent = NULL;

VOID SvcInstall(void);
VOID SvcUninstall(void);
VOID WINAPI SvcCtrlHandler(DWORD);
VOID WINAPI SvcMain(DWORD, LPTSTR *);

VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR *);
VOID SvcReportEvent(LPTSTR);

typedef BOOL (*TypedefSetGlobalHook)();
typedef BOOL (*TypedefUnsetGlobalHook)();
HMODULE hDll = NULL;
TypedefSetGlobalHook SetGlobalHook = NULL;
TypedefUnsetGlobalHook UnsetGlobalHook = NULL;

TCHAR *TGetUnquotedAbsPath(TCHAR *relativePath)
{
    TCHAR *unquotedPath = (TCHAR *)malloc((MAX_PATH - 2) * sizeof(TCHAR));
    GetModuleFileName(NULL, unquotedPath, MAX_PATH - 2);
    TCHAR *lastSlash = _tcsrchr(unquotedPath, TEXT('\\'));
    if (lastSlash != NULL)
    {
        *(lastSlash + 1) = TEXT('\0');
    }
    StringCbCat(unquotedPath, MAX_PATH - 2, relativePath);
    // In case the path contains a space, it must be quoted so that
    // it is correctly interpreted. For example,
    // "d:\my share\myservice.exe" should be specified as
    // ""d:\my share\myservice.exe"".
    return unquotedPath;
}

TCHAR *TGetQuotedAbsPath(TCHAR *relativePath)
{
    TCHAR *quotedPath = (TCHAR *)malloc(MAX_PATH * sizeof(TCHAR));
    TCHAR *unquotedPath = TGetUnquotedAbsPath(relativePath);
    StringCbPrintf(quotedPath, MAX_PATH, TEXT("\"%s\""), unquotedPath);
    free(unquotedPath);
    return quotedPath;
}

void TLogFile(const TCHAR *format, ...)
{
    TCHAR *path = TGetUnquotedAbsPath(TEXT("log.txt"));
    FILE *fp = _tfopen(path, TEXT("a"));
    if (fp == NULL)
    {
        printf("open file error\n");
        return;
    }
    va_list args;
    va_start(args, format);
    _vftprintf(fp, format, args);
    va_end(args);
    fclose(fp);
}

//
// Purpose:
//   Entry point for the process
//
// Parameters:
//   None
//
// Return value:
//   None, defaults to 0 (zero)
//
int __cdecl _tmain(int argc, TCHAR *argv[])
{
    // If command-line parameter is "install", install the service.
    // Otherwise, the service is probably being started by the SCM.

    if (lstrcmpi(argv[1], TEXT("install")) == 0)
    {
        SvcInstall();
        return 0;
    }
    else if (lstrcmpi(argv[1], TEXT("uninstall")) == 0)
    {
        SvcUninstall();
        return 0;
    }
    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] =
        {
            {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)SvcMain},
            {NULL, NULL}};

    // This call returns when the service has stopped.
    // The process should simply terminate when the call returns.
    if (!StartServiceCtrlDispatcher(DispatchTable))
    {
        SvcReportEvent(TEXT("StartServiceCtrlDispatcher"));
    }
}

//
// Purpose:
//   Installs a service in the SCM database
//
// Parameters:
//   None
//
// Return value:
//   None
//
VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR *szPath = TGetQuotedAbsPath(SERVICE_NAME".exe");
    // Get a handle to the SCM database.

    schSCManager = OpenSCManager(
        NULL,                   // local computer
        NULL,                   // ServicesActive database
        SC_MANAGER_ALL_ACCESS); // full access rights

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Create the service

    schService = CreateService(
        schSCManager,              // SCM database
        SERVICE_NAME,              // name of service
        SERVICE_NAME,              // service name to display
        SERVICE_ALL_ACCESS,        // desired access
        SERVICE_WIN32_OWN_PROCESS, // service type
        SERVICE_AUTO_START,        // start type
        SERVICE_ERROR_NORMAL,      // error control type
        szPath,                    // path to service's binary
        NULL,                      // no load ordering group
        NULL,                      // no tag identifier
        NULL,                      // no dependencies
        NULL,                      // LocalSystem account
        NULL);                     // no password

    if (schService == NULL)
    {
        printf("CreateService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }
    else
        printf("Service installed successfully\n");

    if (StartService(schService, 0, NULL))
    {

        printf("Service Started Running\n");
    }
    else
    {
        printf("Service Running Failed (%d)\n", GetLastError());
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return;
}

VOID SvcUninstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database.

    schSCManager = OpenSCManager(
        NULL,                   // local computer
        NULL,                   // ServicesActive database
        SC_MANAGER_ALL_ACCESS); // full access rights

    if (NULL == schSCManager)
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    // Get a handle to the service.

    schService = OpenService(
        schSCManager, // SCM database
        SERVICE_NAME, // name of service
        DELETE);      // need delete access

    if (schService == NULL)
    {
        printf("OpenService failed (%d)\n", GetLastError());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Delete the service.

    if (!DeleteService(schService))
    {
        printf("DeleteService failed (%d)\n", GetLastError());
    }
    else
        printf("Service uninstalled successfully\n");

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

//
// Purpose:
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
//
// Return value:
//   None.
//
VOID WINAPI SvcMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    TLogFile("->SvcMain()\n");
    // Register the handler function for the service
    gSvcStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler"));
        return;
    }

    // These SERVICE_STATUS members remain as set here
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwServiceSpecificExitCode = 0;

    // Report initial status to the SCM
    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.
    SvcInit(dwArgc, lpszArgv);
}

//
// Purpose:
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
//
// Return value:
//   None
//
VOID SvcInit(DWORD dwArgc, LPTSTR *lpszArgv)
{
    TLogFile("->SvcInit()\n");
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

    ghSvcStopEvent = CreateEvent(
        NULL,  // default security attributes
        TRUE,  // manual reset event
        FALSE, // not signaled
        NULL); // no name

    if (ghSvcStopEvent == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    // Report running status when initialization is complete.
    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    // TO_DO: Perform work until service stops.
    // 安装钩子
    // TCHAR *dllPath = TGetUnquotedAbsPath("OpenInCurrentScreen.dll");
    // TLogFile("dllPath: %s\n", dllPath);
    hDll = LoadLibrary(SERVICE_NAME".dll");
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }
    TLogFile("LoadLibraryW: %d\n", hDll);
    SetGlobalHook = (TypedefSetGlobalHook)GetProcAddress(hDll, "SetHook");
    UnsetGlobalHook = (TypedefUnsetGlobalHook)GetProcAddress(hDll, "UnsetHook");
    if (SetGlobalHook == NULL || UnsetGlobalHook == NULL)
    {
        ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }
    SetGlobalHook();
    while (1)
    {
        // Check whether to stop the service.
        // WaitForSingleObject(ghSvcStopEvent, INFINITE);
        // ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        // return;
        Sleep(1000);
    }
}

//
// Purpose:
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation,
//     in milliseconds
//
// Return value:
//   None
//
VOID ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    if (dwWin32ExitCode != NO_ERROR)
    {
        TLogFile("ERROR: GetLastError()=%d\n", dwWin32ExitCode);
    }
    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else
        gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // switch (gSvcStatus.dwCurrentState)
    // {
    // case SERVICE_RUNNING:
    //     TLogFile("Set SERVICE_RUNNING\n");
    //     break;
    // case SERVICE_STOPPED:
    //     TLogFile("Set SERVICE_STOPPED\n");
    //     break;
    // case SERVICE_STOP_PENDING:
    //     TLogFile("Set SERVICE_STOP_PENDING\n");
    //     break;
    // case SERVICE_START_PENDING:
    //     TLogFile("Set SERVICE_START_PENDING\n");
    //     break;
    // default:
    //     TLogFile("Set SERVICE_Status: %d\n", gSvcStatus.dwCurrentState);
    //     break;
    // }

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

//
// Purpose:
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
//
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code.

    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        // Signal the service to stop.

        // 卸载钩子
        UnsetGlobalHook();

        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

        return;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }
}

//
// Purpose:
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
//
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SERVICE_NAME);

    if (NULL != hEventSource)
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SERVICE_NAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    SVC_ERROR,           // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}
