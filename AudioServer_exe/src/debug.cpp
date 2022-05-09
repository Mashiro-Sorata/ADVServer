#include "../include/debug.h"
#include <sstream>

static HMODULE GetSelfModuleHandle()
{
	MEMORY_BASIC_INFORMATION mbi;
	return ((::VirtualQuery(GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0) ?
		(HMODULE)mbi.AllocationBase : NULL);
}

void TCHAR2Char(const TCHAR* tchar, char* _char)
{
	//获取字节长度   
	int iLength;
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

void String2TCHAR(const std::string _str, TCHAR* tchar)
{
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)_str.c_str(), -1, tchar, MAX_CHAR_LENGTH);
}

void GetInstanceFolderPath(std::string* dirPath)
{
	std::string exePath = "";
	TCHAR tcFullPath[MAX_PATH];
	char pChPath[MAX_PATH];
	memset(pChPath, '\0', MAX_PATH);
	/* 获取当前dll的执行路径exe路径 */
	GetModuleFileName(GetSelfModuleHandle(), tcFullPath, MAX_PATH);
	/** 将tchar转为char */
	TCHAR2Char(tcFullPath, pChPath);
	exePath = std::string(pChPath);
	size_t iPos = exePath.rfind("\\");
	*dirPath = exePath.substr(0, iPos + 1);
}

const std::string debug::CLogger::sm_fileName_ = "ADV_Log.log";

const debug::LEVEL debug::CLogger::sm_maxLevel_ = debug::DebugLevel;

std::mutex debug::CLogger::m_logMutex_;

debug::CLogger::CLogger()
{
	m_flag_ = false;
	m_outfile_ = NULL;
}

debug::CLogger::~CLogger()
{
	if (NULL != m_outfile_)
	{
		m_outfile_->close();
		delete m_outfile_;
		m_outfile_ = NULL;
	}
}

void debug::CLogger::Initial(bool flag)
{
	m_flag_ = flag;
	if (m_flag_)
	{
		std::string _dirPath;
		GetInstanceFolderPath(&_dirPath);
		_dirPath += sm_fileName_;
		std::ofstream _temp(_dirPath, std::ios::out);
		_temp.close();
		m_outfile_ = new std::ofstream(_dirPath, std::ios::app);
	}
}

void debug::CLogger::Log_Info(const char* _FILE, const char* _func, const char* format)
{
	if (sm_maxLevel_ >= debug::LEVEL::_INFO_)
		Log_Base(_FILE, _func, debug::LEVEL::_INFO_, "INFO", format);
}

void debug::CLogger::Log_Debug(const char* _FILE, const char* _func, const char* format)
{
	if (sm_maxLevel_ >= debug::LEVEL::_DEBUG_)
		Log_Base(_FILE, _func, debug::LEVEL::_DEBUG_, "DEBUG", format);
}

void debug::CLogger::Log_Warn(const char* _FILE, const char* _func, const char* format)
{
	if (sm_maxLevel_ >= debug::LEVEL::_WARN_)
		Log_Base(_FILE, _func, debug::LEVEL::_WARN_, "WARN", format);
}

void debug::CLogger::Log_Error(const char* _FILE, const char* _func, const char* format)
{
	if (sm_maxLevel_ >= debug::LEVEL::_ERROR_)
		Log_Base(_FILE, _func, debug::LEVEL::_ERROR_, "ERROR", format);
}

void debug::CLogger::Log_Error(const char* _FILE, const char* _func, const char* format, HRESULT code)
{
	if (sm_maxLevel_ >= debug::LEVEL::_ERROR_) {
		char targetString[1024];
		snprintf(targetString,
			sizeof(targetString),
			"%s(0x%.8X)",
			format,
			code);
		Log_Base(_FILE, _func, debug::LEVEL::_ERROR_, "ERROR", targetString);
	}
}

void debug::CLogger::Log_Error(const char* _FILE, const char* _func, const char* format, int code)
{
	if (sm_maxLevel_ >= debug::LEVEL::_ERROR_) {
		char targetString[1024];
		snprintf(targetString,
			sizeof(targetString),
			"%s(%d)",
			format,
			code);
		Log_Base(_FILE, _func, debug::LEVEL::_ERROR_, "ERROR", targetString);
	}
}

debug::CLogger debug::LOGGER = debug::CLogger();