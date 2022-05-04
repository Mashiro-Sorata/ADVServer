/***************************************************************
 * Name:      adv_server.cpp
 * Purpose:   Code for Plug-in CAudioDVServer Class
 * Author:    Mashiro_Sorata (mashiro_sorata@qq.com)
 * Created:   2020-04-29
 * Copyright: Mashiro_Sorata (c)
 * License:   MIT
 **************************************************************/

 /// ============================================================================
 /// declarations
 /// ============================================================================

 /// ----------------------------------------------------------------------------
 /// headers
 /// ----------------------------------------------------------------------------


#include <windows.h>
#include <cstdlib>
#include <string>
#include <tchar.h>
#include"tlhelp32.h"
#pragma comment(lib, "User32.lib")

//隐藏窗口
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

#include "../include/a2fft_server.h"
#include "../include/config.h"
#include "../include/debug.h"

#pragma comment(lib, "Ole32.lib")


int is_su_alive(void);
int Program_Mutex();
void main_app(void);

HANDLE GetProcessHandle(LPCTSTR pName);
HANDLE GetProcessHandle(int nID);

/// ----------------------------------------------------------------------------
/// variables
/// ----------------------------------------------------------------------------
CA2FFTServer* audioServer;
char* srv_ip;
u_short srv_port = 5050;
int srv_maxconn = 5;
bool srv_logger = false;



/// ============================================================================
/// implementation
/// ============================================================================

int main(int argc, char* argv[]) {
    TCHAR tProcessName[MAX_PATH];
    char processName[MAX_PATH];
    GetModuleFileName(NULL, tProcessName, MAX_PATH);
    TCHAR2Char(tProcessName, processName);
    std::string sProcessName = std::string(processName);
    sProcessName = sProcessName.substr(sProcessName.find_last_of('\\') + 1);
    std::cout << sProcessName << std::endl;
    String2TCHAR(sProcessName, tProcessName);

    if (argc == 1)
    {
        main_app();
    }
    else if(2 == argc && !strcmp("-reboot", argv[1])) 
    {
        HANDLE ha = GetProcessHandle(tProcessName);
        if (ha)
        {
            TerminateProcess(ha, 0);
        }
        Sleep(100);
        main_app();
    }
    else if (2 == argc && !strcmp("-close", argv[1]))
    {
        HANDLE ha = GetProcessHandle(tProcessName);
        if (ha)
        {
            TerminateProcess(ha, 0);
        }
    }
    return 0;
}

void main_app(void)
{
    if (!Program_Mutex())
    {
        srv_ip = new char[17];
        ReadServerConfig(&srv_ip, &srv_port, &srv_maxconn, &srv_logger);
        LOG_INIT(srv_logger);
        LOG_INFO("Initial CAudioDVServer!");
        LOG_BASE("Server Address", srv_ip);
        LOG_BASE("Server Port", srv_port);
        LOG_BASE("Server Maxconn", srv_maxconn);
        audioServer = new CA2FFTServer(srv_ip, srv_port, srv_maxconn);

        LOG_INFO("Start CAudioDVServer!");
        audioServer->StartServer();

        while (!is_su_alive()) {
        //while(TRUE) {
            Sleep(2000);
        }
        LOG_ERROR("Process \"SAO Utils\" is not found! Closing ADVServer! ");
        LOG_INFO("Exit CAudioDVServer!");
        audioServer->ExitServer();
        delete[] srv_ip;
        delete audioServer;
    }
}

int is_su_alive(void) {
    if (FindWindow(NULL, L"SAO Utils - Launcher"))
    {
        return 0;
    }
    return 1;
}

//

//函数名：Program_Mutex
//功能：确保程序只有唯一的实例

//返回值：0-正常；1-已经有一个正在运行的实例；-1 -创建互斥对象失败
//
int Program_Mutex()
{
    HANDLE hMutex = NULL;
    LPCWSTR lpszName = L"Mashiro_Sorata_AudioDataVisualizationServer_Mutext";
    int nRet = 0;
    hMutex = ::CreateMutex(NULL,FALSE,lpszName);
    DWORD dwRet = ::GetLastError();

    switch (dwRet)
    {
        case 0:
        {
            break;
        }
        case ERROR_ALREADY_EXISTS:
        {
            LOG_WARN("该应用程序已经开启");
            nRet = 1;
            break;
        }
        default:
        {
            LOG_ERROR("该应用程序创建互斥对象失败");
            nRet = -1;
            break;
        }
    }
    return nRet;
}

HANDLE GetProcessHandle(int nID)
{
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, nID);
}

HANDLE GetProcessHandle(LPCTSTR pName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) {
        return NULL;
    }
    PROCESSENTRY32 pe = { sizeof(pe) };
    BOOL fOk;
    for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe)) {
        if (!_tcscmp(pe.szExeFile, pName)) {
            int pid = (int)getpid();
            if (pe.th32ProcessID != pid) 
            {
                CloseHandle(hSnapshot);
                return GetProcessHandle(pe.th32ProcessID);
            }
        }
    }
    return NULL;
}