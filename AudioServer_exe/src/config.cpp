#include "../include/config.h"
#include "../include/a2fft_server.h"
#include "../include/debug.h"

#define _NVG_TEXT(_TEXT) L##_TEXT
#define NVG_TEXT(_TEXT) _NVG_TEXT(_TEXT)


void ReadServerConfig(char** _ip, unsigned short* _port, int* _maxClient, bool* _logger)
{
    std::string dirPath;
    GetInstanceFolderPath(&dirPath);
    TCHAR wdirPath[MAX_PATH];
    TCHAR w_temp[17];
    String2TCHAR(dirPath + CONFIGFILE, wdirPath);
    GetPrivateProfileString(NVG_TEXT("server"), NVG_TEXT("ip"), NVG_TEXT(DEFAULT_IP_LOCAL), w_temp, 17, wdirPath);
    if (lstrcmpi(NVG_TEXT(DEFAULT_IP_LOCAL), w_temp) && lstrcmpi(NVG_TEXT("localhost"), w_temp))
    {
        strcpy(*_ip, DEFAULT_IP_ANY);
    }
    else
    {
        strcpy(*_ip, DEFAULT_IP_LOCAL);
    }
    *_port = GetPrivateProfileInt(NVG_TEXT("server"), NVG_TEXT("port"), DEFAULT_PORT, wdirPath);
    *_maxClient = GetPrivateProfileInt(NVG_TEXT("server"), NVG_TEXT("maxclient"), DEFAULT_MAXCLIENTS, wdirPath);
    GetPrivateProfileString(NVG_TEXT("server"), NVG_TEXT("logger"), NVG_TEXT("true"), w_temp, 17, wdirPath);
    if (lstrcmpi(NVG_TEXT("true"), w_temp))
    {
        *_logger = false;
    }
    else
    {
        *_logger = true;
    }
}


void ReadFFTConfig(int* fftAttack, int* fftDecay, int* normalizeSpeed, int* peakthr, int* fps, int* changeSpeed) 
{
    std::string dirPath;
    GetInstanceFolderPath(&dirPath);
    TCHAR wdirPath[MAX_PATH];
    String2TCHAR(dirPath + CONFIGFILE, wdirPath);
    *fftAttack = GetPrivateProfileInt(NVG_TEXT("fft"), NVG_TEXT("attack"), 5, wdirPath);
    *fftDecay = GetPrivateProfileInt(NVG_TEXT("fft"), NVG_TEXT("decay"), 5, wdirPath);
    *normalizeSpeed = GetPrivateProfileInt(NVG_TEXT("fft"), NVG_TEXT("norspeed"), 1, wdirPath);
    *peakthr = GetPrivateProfileInt(NVG_TEXT("fft"), NVG_TEXT("peakthr"), 10, wdirPath);
    *fps = GetPrivateProfileInt(NVG_TEXT("fft"), NVG_TEXT("fps"), 35, wdirPath);
    *changeSpeed = GetPrivateProfileInt(NVG_TEXT("fft"), NVG_TEXT("changeSpeed"), 20, wdirPath);
}

//void ReadAdvancedConfig(bool* device_reboot)
//{
//    std::string dirPath;
//    GetInstanceFolderPath(&dirPath);
//    TCHAR wdirPath[MAX_PATH];
//    TCHAR w_temp[17];
//    String2TCHAR(dirPath + CONFIGFILE, wdirPath);
//    GetPrivateProfileString(NVG_TEXT("advanced"), NVG_TEXT("device_reboot"), NVG_TEXT("true"), w_temp, 17, wdirPath);
//    if (lstrcmpi(NVG_TEXT("true"), w_temp))
//    {
//        *device_reboot = false;
//    }
//    else
//    {
//        *device_reboot = true;
//    }
//}