#include "SystemLog.h"
#include <strsafe.h>
#include <iostream>
#include <tchar.h>
#include <time.h>

SystemLog* g_pSystemLog = SystemLog::GetInstance();

SystemLog::SystemLog() : _dir(nullptr), _logLevel(ERROR_LEVEL)
{

}

SystemLog* SystemLog::GetInstance()
{
	static SystemLog _systemLog;
	return &_systemLog;
}

void SystemLog::SetSysLogDir(const wchar_t* dir)
{
	if (_dir != nullptr)
		delete[] _dir;

	int dirLen = wcslen(dir);
	_dir = new WCHAR[dirLen + 1];
	HRESULT ret = StringCchCopy(_dir, dirLen + 1, dir);
	_dir[dirLen] = L'\0';

	if (FAILED(ret))
	{
		::wprintf(L"Fail to StringCchCopy : %d (%s[%d])\n",
			ret, _T(__FUNCTION__), __LINE__);
		return;
	}

	CreateDirectory(_dir, NULL);
}

void SystemLog::SetSysLogLevel(LOG_LEVEL logLevel)
{
	_logLevel = logLevel;
}

void SystemLog::LogHex(const wchar_t* type, LOG_LEVEL logLevel, const wchar_t* log, BYTE* pByte, int byteLen)
{
	if (logLevel < _logLevel) return;

	time_t baseTimer = time(NULL);
	struct tm localTimer;
	localtime_s(&localTimer, &baseTimer);

	WCHAR logTitle[dfTITLE_LEN] = { '\0' };
	HRESULT titleRet = StringCchPrintf(logTitle, dfTITLE_LEN, L"%s/%d%02d_%s_LogHex.txt",
		_dir, localTimer.tm_year + 1900, localTimer.tm_mon + 1, type);
	logTitle[dfTITLE_LEN - 1] = L'\0';

	if (FAILED(titleRet))
	{
		::wprintf(L"Fail to StringCchPrintf : %d (%s[%d])\n",
			titleRet, _T(__FUNCTION__), __LINE__);
		return;
	}

	WCHAR logInfo[dfINFO_LEN] = { '\0', };
	HRESULT InfoRet = StringCchPrintf(logInfo, dfINFO_LEN,
		L"[%s][%d-%02d-%02d %02d:%02d:%02d/level:%d/count:%08llu] ", type,
		localTimer.tm_year + 1900, localTimer.tm_mon + 1, localTimer.tm_mday,
		localTimer.tm_hour, localTimer.tm_min, localTimer.tm_sec,
		logLevel, _logCount++);
	logInfo[dfINFO_LEN - 1] = L'\0';

	if (FAILED(InfoRet))
	{
		::wprintf(L"Fail to StringCchPrintf : %d (%s[%d])\n",
			InfoRet, _T(__FUNCTION__), __LINE__);
		return;
	}

	WCHAR logData[dfDATA_LEN] = { '\0', };
	HRESULT copyRet = StringCchCopy(logData, dfDATA_LEN, log);
	if (FAILED(copyRet))
	{
		::wprintf(L"Fail to StringCchCat : %d (%s[%d])\n",
			copyRet, _T(__FUNCTION__), __LINE__);
		return;
	}

	HRESULT concatRet1 = StringCchCat(logData, dfDATA_LEN, L"\n");
	if (FAILED(concatRet1))
	{
		::wprintf(L"Fail to StringCchCat : %d (%s[%d])\n",
			concatRet1, _T(__FUNCTION__), __LINE__);
		return;
	}

	for (int i = 0; i < byteLen; i++)
	{
		WCHAR logHexData[dfHEXDATA_LEN] = { '\0', };
		HRESULT printRet = StringCchPrintf(logHexData, dfHEXDATA_LEN, L"%02x ", pByte[i]);
		if (FAILED(printRet))
		{
			::wprintf(L"Fail to StringCchPrintf : %d (%s[%d])\n",
				printRet, _T(__FUNCTION__), __LINE__);
			return;
		}

		HRESULT concatRet2 = StringCchCat(logData, dfDATA_LEN, logHexData);
		if (FAILED(concatRet2))
		{
			::wprintf(L"Fail to StringCchCat : %d (%s[%d])\n",
				concatRet2, _T(__FUNCTION__), __LINE__);
			return;
		}
	}

	logData[dfDATA_LEN - 1] = L'\0';

	FILE* file;
	errno_t openRet = _wfopen_s(&file, logTitle, L"a+");
	if (openRet != 0)
	{
		::wprintf(L"Fail to open %s : %d (%s[%d])\n",
			logTitle, openRet, _T(__FUNCTION__), __LINE__);
		return;
	}

	if (file != nullptr)
	{
		fwprintf(file, logInfo);
		fwprintf(file, logData);
		fclose(file);
	}
	else
	{
		::wprintf(L"Fileptr is nullptr %s (%s[%d])\n",
			logTitle, _T(__FUNCTION__), __LINE__);
		return;
	}
}

void SystemLog::Log(const wchar_t* type, LOG_LEVEL logLevel, const wchar_t* stringFormat, ...)
{
	if (logLevel < _logLevel) return;

	time_t baseTimer = time(NULL);
	struct tm localTimer;
	localtime_s(&localTimer, &baseTimer);

	WCHAR logTitle[dfTITLE_LEN] = { '\0' };
	HRESULT titleRet = StringCchPrintf(logTitle, dfTITLE_LEN, L"%s/%d%02d_%s_Log.txt",
		_dir, localTimer.tm_year + 1900, localTimer.tm_mon + 1, type);
	logTitle[dfTITLE_LEN - 1] = L'\0';

	if (FAILED(titleRet))
	{
		::wprintf(L"Fail to StringCchPrintf : %d (%s[%d])\n",
			titleRet, _T(__FUNCTION__), __LINE__);
		return;
	}

	WCHAR logInfo[dfINFO_LEN] = { '\0', };
	HRESULT InfoRet = StringCchPrintf(logInfo, dfINFO_LEN,
		L"[%s][%d-%02d-%02d %02d:%02d:%02d/level:%d/count:%08llu] ", type,
		localTimer.tm_year + 1900, localTimer.tm_mon + 1, localTimer.tm_mday,
		localTimer.tm_hour, localTimer.tm_min, localTimer.tm_sec,
		logLevel, _logCount++);
	logInfo[dfINFO_LEN - 1] = L'\0';

	if (FAILED(InfoRet))
	{
		::wprintf(L"Fail to StringCchPrintf : %d (%s[%d])\n",
			InfoRet, _T(__FUNCTION__), __LINE__);
		return;
	}

	WCHAR logData[dfDATA_LEN] = { '\0', };
	va_list va;
	va_start(va, stringFormat);
	HRESULT dataRet = StringCchVPrintf(logData, dfDATA_LEN, stringFormat, va);
	logData[dfDATA_LEN - 1] = L'\0';
	va_end(va);

	if (FAILED(dataRet))
	{
		::wprintf(L"Fail to StringCchVPrintf : %d (%s[%d])\n",
			dataRet, _T(__FUNCTION__), __LINE__);
		return;
	}

	FILE* file;
	errno_t openRet = _wfopen_s(&file, logTitle, L"a+");
	if (openRet != 0)
	{
		::wprintf(L"Fail to open %s : %d (%s[%d])\n",
			logTitle, openRet, _T(__FUNCTION__), __LINE__);
		return;
	}

	if (file != nullptr)
	{
		fwprintf(file, logInfo);
		fwprintf(file, logData);
		fclose(file);
	}
	else
	{
		::wprintf(L"Fileptr is nullptr %s (%s[%d])\n",
			logTitle, _T(__FUNCTION__), __LINE__);
		return;
	}
}