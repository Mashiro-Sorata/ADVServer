#pragma once

#ifndef DEBUG_H
#define DEBUG_H

#include <windows.h>
#include <iostream>

#define MAX_CHAR_LENGTH 512

void String2TCHAR(const std::string _str, TCHAR* tchar);
void GetInstanceFolderPath(std::string* dirPath);
void TCHAR2Char(const TCHAR* tchar, char* _char);

#include<mutex>
#include <fstream>


namespace debug
{
	enum class LEVEL { _ERROR_ = 0, _WARN_ = 1, _INFO_ = 2, _DEBUG_ = 3, _ALL_ = 4 };

	//控制输出等级
	const LEVEL DebugLevel(LEVEL::_ALL_);

	class CLogger
	{
	public:
		CLogger();
		~CLogger();
		void Initial(bool flag);
		void Log_Info(const char* _FILE, const char* _func, const char* format);
		void Log_Debug(const char* _FILE, const char* _func, const char* format);
		void Log_Warn(const char* _FILE, const char* _func, const char* format);
		void Log_Error(const char* _FILE, const char* _func, const char* format);
		void Log_Error(const char* _FILE, const char* _func, const char* format, HRESULT code);
		void Log_Error(const char* _FILE, const char* _func, const char* format, int code);
		template<typename T> void Log_Base(const char* _FILE, const char* _func, LEVEL level, const char* name, T format);

	private:
		static const std::string sm_fileName_;
		std::ofstream* m_outfile_;
		bool m_flag_;
		static const LEVEL sm_maxLevel_;
		SYSTEMTIME m_now_;
		static std::mutex m_logMutex_;
	};

	template<typename T>
	void CLogger::Log_Base(const char* _FILE, const char* _func, LEVEL level, const char* name, T format)
	{
		if (m_flag_)
		{
			GetLocalTime(&m_now_);
			char sdata[5];
			std::string _headLog;
			_headLog.append("<");
			_itoa_s(m_now_.wHour, sdata, 10);
			_headLog.append(2 - strlen(sdata), '0');
			_headLog.append(sdata);
			_headLog.append(":");
			_itoa_s(m_now_.wMinute, sdata, 10);
			_headLog.append(2 - strlen(sdata), '0');
			_headLog.append(sdata);
			_headLog.append(":");
			_itoa_s(m_now_.wSecond, sdata, 10);
			_headLog.append(2 - strlen(sdata), '0');
			_headLog.append(sdata);
			_headLog.append(">");
			_headLog.append("[");
			_headLog.append(_FILE);
			_headLog.append("](Function: ");
			_headLog.append(_func);
			_headLog.append(")>>>>");
			_headLog.append(name);
			_headLog.append(": ");
			m_logMutex_.lock();
			(*m_outfile_) << _headLog << format << std::endl;
			m_logMutex_.unlock();
		}
		
	}
	extern CLogger LOGGER;
}

#define LOG_INIT(_flag) debug::LOGGER.Initial(_flag)
#define LOG_INFO(format) debug::LOGGER.Log_Info(__FILE__, __func__, format)
#define LOG_DEBUG(format) debug::LOGGER.Log_Debug(__FILE__, __func__, format)
#define LOG_WARN(format) debug::LOGGER.Log_Warn(__FILE__, __func__, format)
#define LOG_ERROR(format) debug::LOGGER.Log_Error(__FILE__, __func__, format)
#define LOG_ERROR_CODE(format, code) debug::LOGGER.Log_Error(__FILE__, __func__, format, code)
#define LOG_BASE(name, format) debug::LOGGER.Log_Base(__FILE__, __func__, debug::LEVEL::_ALL_, name, format)

#endif // !DEBUG_H

