#include "MyLog.h"

#include <Windows.h>

#include <stdlib.h>
#include <time.h>
#include <wchar.h>

class Logger
{
  public:
    Logger()
    {
        time_t t = time(NULL);
        tm localTime;
        errno_t err;

        WCHAR curPath[MAX_PATH];
        int ret1 = GetCurrentDirectoryW(_countof(curPath), curPath);
        if (ret1 == 0)
        {
            wprintf(L"GetCurrentDirectoryW() err in Logger\n");
            abort();
        }

        errno_t ret2 = wcscat_s(curPath, _countof(curPath), L"\\Log");
        if (ret2 != 0)
        {
            wprintf(L"wcscat_s() err in Logger\n");
            abort();
        }

        int ret3 = _wmkdir(curPath);
        if (ret3 == -1 && errno != EEXIST)
        {
            wprintf(L"_wmkdir() err in Logger\n");
            abort();
        }

        err = localtime_s(&localTime, &t);
        if (err != 0)
        {
            wprintf(L"localtime_s err in Logger\n");
            abort();
        }
        int result = swprintf_s(Filename, _countof(Filename), L"%s\\Log_%04d%02d%02d_%02d%02d%02d.txt", curPath,
                                localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour,
                                localTime.tm_min, localTime.tm_sec);
        if (result == -1)
        {
            wprintf(L"sprintf_s err in Logger\n");
            abort();
        }

        FILE* pFile = nullptr;
        _wfopen_s(&pFile, Filename, L"w");
        if (pFile == nullptr)
        {
            wprintf(L"fopen_s err in Logger\n");
            abort();
        }

        fclose(pFile);
    }

  public:
    wchar_t Filename[MAX_PATH];
};

static Logger s_logger;

FILE* _GetLogFilePtr()
{
    FILE* pFile;
    _wfopen_s(&pFile, s_logger.Filename, L"a+");
    return pFile;
}