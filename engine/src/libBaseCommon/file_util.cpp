#include "file_util.h"

#include <string>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi")
#else
#include <unistd.h>
#include <dirent.h>
#endif

namespace
{
	void modifyDirectoryPath(std::string& szPath) // 修改目录路径为X/Y/Z/
	{
		if (szPath.empty())
			return;

		for (std::string::iterator iter = szPath.begin(); iter != szPath.end(); ++iter)
		{
			if (*iter == '\\')
			{
				*iter = '/';
			}
		}

		if (szPath.at(szPath.length() - 1) != '/')
		{
			szPath += "/";
		}
	}
}

namespace base
{
	namespace file_util
	{
		bool isDirectory(const char* szDir)
		{
			if (szDir == nullptr)
				return false;

#ifdef _WIN32
			return ::PathIsDirectoryA(szDir) ? true : false;
#else
			DIR* pDir = ::opendir(szDir);
			if (pDir == nullptr)
				return false;

			::closedir(pDir);
			return true;
#endif
		}

		bool createRecursionDir(const char* szDir)
		{
			if (nullptr == szDir)
				return false;

			std::string szRealDir(szDir);
			if (szRealDir.empty())
				return true;

			modifyDirectoryPath(szRealDir);

			std::string::size_type pos = szRealDir.find('/');
			while (pos != std::string::npos)
			{
				std::string nPos = szRealDir.substr(0, pos - 0);
				if (nPos.length() > 0 && !isDirectory(nPos.c_str()))
				{
					bool bRet = false;
#ifdef _WIN32
					bRet = CreateDirectoryA(nPos.c_str(), NULL) ? true : false;
#else
					bRet = (mkdir(nPos.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0);
#endif
					if (!bRet)
					{
						return false;
					}
				}

				pos = szRealDir.find('/', pos + 1);
			}

			return true;
		}

		bool isFileExist(const char* szFileName)
		{
			if (nullptr == szFileName)
				return false;

			FILE *pFile = fopen(szFileName, "rb");
			if (pFile == nullptr)
				return false;

			::fclose(pFile);
			return true;
		}

		int64_t getFileSize(const char* szFileName)
		{
#ifdef _WIN32
			struct _stat st;       // see : https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
			if (_stat(szFileName, &st) != 0)
				return -1;

			return st.st_size;
#else
			struct stat st;
			if (::stat(szFileName, &st) != 0)
				return -1;

			return st.st_size;
#endif
		}

		size_t readFile(const char* szFileName, char* szBuf, size_t nBufSize)
		{
			if (nullptr == szFileName || szBuf == nullptr)
				return 0;

			FILE* pFile = fopen(szFileName, "rb");
			if (pFile == nullptr)
				return 0;

			const static size_t PER_READ_SIZE = 4096;
			size_t nTotal = 0;
			size_t nPos = 0;
			char szTmpBuf[PER_READ_SIZE];
			while (!feof(pFile))
			{
				size_t nSize = fread(szTmpBuf, 1, PER_READ_SIZE, pFile);
				if (nPos + nSize > nBufSize)
				{
					memcpy(szBuf + nPos, szTmpBuf, nBufSize - nPos);
					nTotal += nBufSize - nPos;
					break;
				}

				memcpy(szBuf + nPos, szTmpBuf, nSize);
				nTotal += nSize;
				if (nSize < PER_READ_SIZE)
					break;
			}

			fclose(pFile);

			return nTotal;
		}
	}
}