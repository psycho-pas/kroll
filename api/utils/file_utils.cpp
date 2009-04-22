/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "utils.h"

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <sys/utsname.h>
#include <libgen.h>
#elif defined(OS_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <Iphlpapi.h>
#include <process.h>
#elif defined(OS_LINUX)
#include <cstdarg>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <sys/utsname.h>
#include <libgen.h>
#endif

#include <iostream>
#include <sstream>
#include <cstring>

const std::string ILLEGAL = "<>{}|\\\"^`";

static std::string safe_encode(std::string &str)
{
	std::string encodedStr;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		char c = *it;
		if (isalnum(c) ||
			c == '-' || c == '_' ||
			c == '.' || c == '~' ||
			c == '/' || c == '\\' ||
			c == ' ')
		{
			encodedStr += c;
		}
		else if (c == ' ')
		{
			encodedStr += c;
		}
		else if (c <= 0x20 || c >= 0x7F || ILLEGAL.find(c) != std::string::npos)
		{
			// skip these bad out of range characters ....
		}
		else encodedStr += c;
	}
	return encodedStr;
}


namespace kroll
{
	const char HEX2DEC[256] =
	{
		/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
		/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

		/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

		/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

		/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
	};
	// Only alphanum is safe.
	const char SAFE[256] =
	{
		/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
		/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

		/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
		/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
		/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
		/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

		/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

		/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
	};

	static bool CompareVersions(std::string a, std::string b)
	{
		int a1 = FileUtils::MakeVersion(a);
		int b1 = FileUtils::MakeVersion(b);
		return b1 > a1;
	}
	std::string FileUtils::GetApplicationDirectory()
	{
#ifdef OS_OSX
		NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
		NSString* contents = [NSString stringWithFormat:@"%@/Contents",bundlePath];
		return std::string([contents UTF8String]);
#elif OS_WIN32
		char path[MAX_PATH];
		GetModuleFileName(NULL,path,MAX_PATH);
		std::string p(path);
		std::string::size_type pos = p.rfind("\\");
		if (pos!=std::string::npos)
		{
		  return p.substr(0,pos);
		}
	  	return p;
#elif OS_LINUX
		char tmp[100];
		sprintf(tmp,"/proc/%d/exe",getpid());
		char pbuf[255];
		int c = readlink(tmp,pbuf,255);
		pbuf[c]='\0';
		std::string str(pbuf);
		size_t pos = str.rfind("/");
		if (pos==std::string::npos) return str;
		return str.substr(0,pos);
#endif
	}

	std::string FileUtils::GetApplicationDataDirectory(std::string &appid)
	{
		std::string dir = GetUserRuntimeHomeDirectory();
		dir.append(KR_PATH_SEP);
		dir.append("appdata");
		if (!IsDirectory(dir))
		{
			CreateDirectory(dir);
		}
		dir.append(KR_PATH_SEP);
		dir.append(appid);
		if (!IsDirectory(dir))
		{
			CreateDirectory(dir);
		}
		return dir;
	}
	std::string FileUtils::GetTempDirectory()
	{
#ifdef OS_OSX
		NSString * tempDir = NSTemporaryDirectory();
		if (tempDir == nil)
		    tempDir = @"/tmp";

		NSString *tmp = [tempDir stringByAppendingPathComponent:@"kXXXXX"];
		const char * fsTemplate = [tmp fileSystemRepresentation];
		NSMutableData * bufferData = [NSMutableData dataWithBytes: fsTemplate
		                                                   length: strlen(fsTemplate)+1];
		char * buffer = (char*)[bufferData mutableBytes];
		mkdtemp(buffer);
		NSString * temporaryDirectory = [[NSFileManager defaultManager]
		        stringWithFileSystemRepresentation: buffer
		                                    length: strlen(buffer)];
		return std::string([temporaryDirectory UTF8String]);
#elif defined(OS_WIN32)
#define BUFSIZE 512
		TCHAR szTempName[BUFSIZE];
		GetTempPath(BUFSIZE,szTempName);
		std::ostringstream s;
		srand(GetTickCount()); // initialize seed
		std::string dir(szTempName);
		s << dir;
		s << "\\k";
		s << (double)rand();
		return s.str();
#else
		std::ostringstream dir;
		const char* tmp = getenv("TMPDIR");
		const char* tmp2 = getenv("TEMP");
		if (tmp)
			dir << std::string(tmp);
		else if (tmp2)
			dir << std::string(tmp2);
		else
			dir << std::string("/tmp");

		std::string tmp_str = dir.str();
		if (tmp_str.at(tmp_str.length()-1) != '/')
			dir << "/";
		dir << "kXXXXXX";
		char* tempdir = strdup(dir.str().c_str());
		tempdir = mkdtemp(tempdir);
		tmp_str = std::string(tempdir);
		free(tempdir);
		return tmp_str;
#endif
	}
	std::string FileUtils::GetResourcesDirectory()
	{
#ifdef OS_OSX
		NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
		std::string dir = std::string([resourcePath UTF8String]);
#elif OS_WIN32
		std::string dir = FileUtils::GetApplicationDirectory();
		dir.append("\\Resources");
#elif OS_LINUX
		// TODO test this
		std::string dir = FileUtils::GetApplicationDirectory();
		dir.append("/Resources");
#endif
		return dir;
	}
	bool FileUtils::IsFile(std::string &file)
	{
#ifdef OS_OSX
		BOOL isDir = NO;
		NSString *f = [NSString stringWithCString:file.c_str()];
		NSString *p = [f stringByStandardizingPath];
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:p isDirectory:&isDir];
		return found && !isDir;
#elif OS_WIN32
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(file.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000000) == 0x00000000;
			FindClose(hFind);
			return yesno;
		}
		return false;
#elif OS_LINUX
		struct stat st;
		return (stat(file.c_str(),&st)==0) && S_ISREG(st.st_mode);
#endif
	}
	std::string FileUtils::Dirname(std::string path)
	{
#ifdef OS_WIN32
	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	strncpy(path_buffer, path.c_str(), _MAX_PATH);
	_splitpath(path_buffer, drive, dir, fname, ext );
	return std::string(dir);
#else
	char* pathCopy = strdup(path.c_str());
	std::string toReturn = dirname(pathCopy);
	free(pathCopy);
	return toReturn;
#endif
	}
	bool FileUtils::CreateDirectory(std::string &dir)
	{
#ifdef OS_OSX
		return [[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithCString:dir.c_str()] attributes:nil];
#elif OS_WIN32
		return ::CreateDirectory(dir.c_str(),NULL);
#elif OS_LINUX
		return mkdir(dir.c_str(),0755) == 0;
#endif
		return false;
	}
	bool FileUtils::CreateDirectory2(std::string &dir)
	{
		return FileUtils::CreateDirectory(dir);
	}
	bool FileUtils::DeleteDirectory(std::string &dir)
	{
#ifdef OS_OSX
		[[NSFileManager defaultManager] removeFileAtPath:[NSString stringWithCString:dir.c_str()] handler:nil];
#elif OS_WIN32
		SHFILEOPSTRUCT op;
		op.hwnd = NULL;
		op.wFunc = FO_DELETE;
		op.pFrom = dir.c_str();
		op.pTo = NULL;
		op.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
		int rc = SHFileOperation(&op);
		return (rc == 0);
#elif OS_LINUX
		return unlink(dir.c_str()) == 0;
#endif
		return false;
	}
	bool FileUtils::IsDirectory(std::string &dir)
	{
#ifdef OS_OSX
		BOOL isDir = NO;
		BOOL found = [[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithCString:dir.c_str()] isDirectory:&isDir];
		return found && isDir;
#elif OS_WIN32
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(dir.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000010) == 0x00000010;
			FindClose(hFind);
			return yesno;
		}
		return false;
#elif OS_LINUX
		struct stat st;
		return (stat(dir.c_str(),&st)==0) && S_ISDIR(st.st_mode);
#endif
	}

	std::string FileUtils::GetDirectory(std::string &file)
	{
		size_t pos = file.find_last_of(KR_PATH_SEP);
		if (pos == std::string::npos)
		{
			pos = file.find_last_of(KR_PATH_SEP_OTHER);
			if (pos == std::string::npos)
			{
				return "."KR_PATH_SEP; //??
			}
		}
#ifdef OS_OSX
		NSString *s = [[NSString stringWithCString:file.substr(0,pos).c_str()] stringByExpandingTildeInPath];
		return [s fileSystemRepresentation];
#else
		return file.substr(0, pos);
#endif
	}


	std::string FileUtils::Join(const char* path, ...)
	{
		va_list ap;
		va_start(ap, path);
		std::vector<std::string> parts;
		parts.push_back(std::string(path));
		while (true)
		{
			const char *i = va_arg(ap,const char*);
			if (i == NULL)
				break;
			parts.push_back(Trim(i));
		}
		va_end(ap);
		std::string filepath;
		std::vector<std::string>::iterator iter = parts.begin();
		while (iter!=parts.end())
		{
			std::string p = (*iter++);
			filepath += p;

			if (filepath.length() != 0
				&& iter != parts.end()
				&& filepath[filepath.length()] != KR_PATH_SEP[0])
			{
				filepath += KR_PATH_SEP;
			}
		}
#ifdef OS_OSX
		NSString *s = [[NSString stringWithCString:filepath.c_str()] stringByExpandingTildeInPath];
		NSString *p = [s stringByStandardizingPath];
		return std::string([p fileSystemRepresentation]);
#else
		return filepath;
#endif
	}

	bool FileUtils::IsHidden(std::string &file)
	{
#ifdef OS_OSX
		// TODO finish this
		return false;
#elif OS_WIN32
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(file.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			bool yesno = (findFileData.dwFileAttributes & 0x00000002) == 0x00000002;
			FindClose(hFind);
			return yesno;
		}
		return false;
#elif OS_LINUX
		return (file.size() > 0 && file.at(0) == '.');
#endif
	}

	bool FileUtils::IsRuntimeInstalled()
	{
		std::string systemRTH = GetSystemRuntimeHomeDirectory();
		std::string userRTH = GetUserRuntimeHomeDirectory();
		return IsDirectory(systemRTH) || IsDirectory(userRTH);
	}

	void FileUtils::ExtractVersion(std::string& spec, int *op, std::string &version)
	{
		if (spec.find(">=")!=std::string::npos)
		{
			*op = GreaterThanEqualTo;
			version = spec.substr(2,spec.length());
		}
		else if (spec.find("<=")!=std::string::npos)
		{
			*op = LessThanEqualTo;
			version = spec.substr(2,spec.length());
		}
		else if (spec.find("<")!=std::string::npos)
		{
			*op = LessThan;
			version = spec.substr(1,spec.length());
		}
		else if (spec.find(">")!=std::string::npos)
		{
			*op = GreaterThan;
			version = spec.substr(1,spec.length());
		}
		else if (spec.find("=")!=std::string::npos)
		{
			*op = EqualTo;
			version = spec.substr(1,spec.length());
		}
		else
		{
			*op = EqualTo;
			version = spec;
		}
	}
	int FileUtils::MakeVersion(std::string& ver)
	{
		std::string v;
		size_t pos = 0;
		while(1)
		{
			size_t newpos = ver.find(".",pos);
			if (newpos==std::string::npos)
			{
				v.append(ver.substr(pos,ver.length()));
				break;
			}
			v.append(ver.substr(pos,newpos));
			pos = newpos+1;
		}
		return atoi(v.c_str());
	}
	std::string FileUtils::FindVersioned(std::string& path, int op, std::string& version)
	{
		std::vector<std::string> files;
		std::vector<std::string> found;
		ListDir(path,files);
		std::vector<std::string>::iterator iter = files.begin();
		int findVersion = MakeVersion(version);
		while (iter!=files.end())
		{
			std::string str = (*iter++);
			std::string fullpath = std::string(path);
			fullpath.append(KR_PATH_SEP);
			fullpath.append(str);
			if (IsDirectory(fullpath))
			{
				int theVersion = MakeVersion(str);
				bool matched = false;

				switch(op)
				{
					case EqualTo:
					{
						matched = (theVersion == findVersion);
						break;
					}
					case GreaterThanEqualTo:
					{
						matched = (theVersion >= findVersion);
						break;
					}
					case LessThanEqualTo:
					{
						matched = (theVersion <= findVersion);
						break;
					}
					case GreaterThan:
					{
						matched = (theVersion > findVersion);
						break;
					}
					case LessThan:
					{
						matched = (theVersion < findVersion);
						break;
					}
				}
				if (matched)
				{
					found.push_back(std::string(str));
				}
			}
		}
		if (found.size() > 0)
		{
			std::sort(found.begin(),found.end(),CompareVersions);
			std::string file = found.at(0);
			std::string f = std::string(path);
			f.append(KR_PATH_SEP);
			f.append(file);
			return f;
		}
		return std::string();
	}
	std::string FileUtils::GetOSVersion()
	{
#ifdef OS_WIN32
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (GetVersionEx(&vi) == 0) return "?";

		std::ostringstream str;
		str << vi.dwMajorVersion << "." << vi.dwMinorVersion << " (Build " << (vi.dwBuildNumber & 0xFFFF);
		if (vi.szCSDVersion[0]) str << ": " << vi.szCSDVersion;
		str << ")";
		return str.str();
#elif OS_OSX || OS_LINUX
		struct utsname uts;
		uname(&uts);
		return uts.release;
#endif
	}
	std::string FileUtils::GetOSArchitecture()
	{
#ifdef OS_WIN32
		return std::string("win32");
#elif OS_OSX || OS_LINUX
		struct utsname uts;
		uname(&uts);
		return uts.machine;
#endif
	}
	std::string FileUtils::EncodeURIComponent(std::string src)
	{
		const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
		const unsigned char *pSrc = (const unsigned char *)src.c_str();
	   	const int SRC_LEN = src.length();
	   	unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
	   	unsigned char * pEnd = pStart;
	   	const unsigned char * const SRC_END = pSrc + SRC_LEN;

	   	for (; pSrc < SRC_END; ++pSrc)
	   	{
	      if (SAFE[*pSrc])
	         *pEnd++ = *pSrc;
	      else
	      {
	         // escape this char
	         *pEnd++ = '%';
	         *pEnd++ = DEC2HEX[*pSrc >> 4];
	         *pEnd++ = DEC2HEX[*pSrc & 0x0F];
	      }
	   	}

	   	std::string sResult((char *)pStart, (char *)pEnd);
	   	delete [] pStart;

	   	return sResult;
	}
	std::string FileUtils::DecodeURIComponent(std::string src)
	{
		// Note from RFC1630: "Sequences which start with a percent
		// sign but are not followed by two hexadecimal characters
		// (0-9, A-F) are reserved for future extension"

		const unsigned char * pSrc = (const unsigned char *)src.c_str();
		const int SRC_LEN = src.length();
		const unsigned char * const SRC_END = pSrc + SRC_LEN;
		// last decodable '%'
		const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

		char * const pStart = new char[SRC_LEN];
		char * pEnd = pStart;

		while (pSrc < SRC_LAST_DEC)
		{
		   if (*pSrc == '%')
		   {
		      char dec1, dec2;
		      if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
		         && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
		      {
		         *pEnd++ = (dec1 << 4) + dec2;
		         pSrc += 3;
		         continue;
		      }
		   }

		   *pEnd++ = *pSrc++;
		}

		// the last 2- chars
		while (pSrc < SRC_END)
		   *pEnd++ = *pSrc++;

		std::string sResult(pStart, pEnd);
		delete [] pStart;

		return sResult;
	}
	void FileUtils::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string &delimeters, bool skip_if_found)
	{
		std::string::size_type lastPos = str.find_first_not_of(delimeters,0);
		std::string::size_type pos = str.find_first_of(delimeters,lastPos);
		while (std::string::npos!=pos || std::string::npos!=lastPos)
		{
			std::string token = str.substr(lastPos,pos-lastPos);
			bool found = false;
			if (skip_if_found)
			{
				std::vector<std::string>::iterator i = tokens.begin();
				while(i!=tokens.end())
				{
					std::string entry = (*i++);
					if (entry == token)
					{
						found = true;
						break;
					}
				}
			}
			if (!found)
			{
				tokens.push_back(token);
			}
			lastPos = str.find_first_not_of(delimeters,pos);
			pos = str.find_first_of(delimeters,lastPos);
		}
	}
	std::string FileUtils::Trim(std::string str)
	{
		std::string c(safe_encode(str));
		while (1)
		{
			size_t pos = c.rfind(" ");
			if (pos == std::string::npos || pos!=c.length()-1)
			{
				break;
			}
			c = c.substr(0,pos);
		}
		while(1)
		{
			size_t pos = c.find(" ");
			if (pos != 0)
			{
				break;
			}
			c = c.substr(1);
		}
		return c;
	}
	bool FileUtils::ReadManifest(std::string& path, std::string &runtimePath, std::vector< std::pair< std::pair<std::string,std::string>, bool> >& modules, std::vector<std::string> &moduleDirs, std::string &appname, std::string &appid, std::string &runtimeOverride, std::string &guid)
	{
		std::ifstream file(path.c_str());
		if (file.bad() || file.fail())
		{
			return false;
		}
		bool foundRuntime = false;
		const char *rt = runtimeOverride.c_str();
#ifdef DEBUG
				std::cout << "Read Manifest: " << rt << std::endl;
#endif

		while (!file.eof())
		{
			std::string line;
			std::getline(file,line);
			if (line.find(" ")==0)
			{
				continue;
			}
			size_t pos = line.find(":");
			if (pos!=std::string::npos)
			{
				std::string key = Trim(line.substr(0,pos));
				std::string value = Trim(line.substr(pos+1,line.length()));
				if (key == "#appname")
				{
					appname = value;
					continue;
				}
				else if (key == "#appid")
				{
					appid = value;
					continue;
				}
				else if (key == "#guid")
				{
					guid = value;
					continue;
				}
				else if (key.c_str()[0]=='#')
				{
					continue;
				}
				int op;
				std::string version;
				ExtractVersion(value,&op,version);
#ifdef DEBUG
				std::cout << "Component: " << key << ":" << version << ", operation: " << op << std::endl;
#endif
				std::pair<std::string,std::string> p(key,version);
				if (key == "runtime")
				{
					// check to see if our runtime is found in our override directory
					if (!runtimeOverride.empty())
					{
						std::string potentialRuntime = Join(rt,"runtime",NULL);
						if (IsDirectory(potentialRuntime))
						{
							// extra special check for Win32 since we have to place the WebKit.dll
							// inside the same relative path as .exe because of the COM embedded manifest crap-o-la
							// so if we can't find kroll.dll in the resources folder we don't override
#ifdef OS_WIN32
							std::string krolldll = kroll::FileUtils::Join(potentialRuntime.c_str(),"kroll.dll",NULL);
							if (kroll::FileUtils::IsFile(krolldll))
							{
#endif
								runtimePath = potentialRuntime;
#ifdef DEBUG
								std::cout << "found override runtime at: " << runtimePath << std::endl;
#endif
								foundRuntime = true;
								continue;
#ifdef OS_WIN32
							}
#endif
						}
					}
					runtimePath = FindRuntime(op,version);
					if (runtimePath == "") {
						modules.push_back(std::pair< std::pair<std::string,std::string>, bool>(p,false));
					}

					foundRuntime = true; // so we don't add again
				}
				else
				{
					// check to see if our module is contained within our runtime override
					// directory and if it is, use it...
					std::string potentialModule = kroll::FileUtils::Join(rt,"modules",key.c_str(),NULL);

#ifdef DEBUG
					std::cout << "looking for bundled module at " << potentialModule << std::endl;
#endif
					if (IsDirectory(potentialModule))
					{
						modules.push_back(std::pair< std::pair<std::string,std::string>, bool>(p,true));
						moduleDirs.push_back(potentialModule);
#ifdef DEBUG
						std::cout << "found override module at: " << potentialModule << std::endl;
#endif
						continue;
					}
					std::string dir = FindModule(key,op,version);
					bool found = dir!="";
					modules.push_back(std::pair< std::pair<std::string,std::string>, bool>(p,found));
					if (found)
					{
#ifdef DEBUG
						std::cout << "found module at: " << dir << std::endl;
#endif
						moduleDirs.push_back(dir);
					}
#ifdef DEBUG
					else
					{
						std::cerr << "couldn't find module module: " << key  << "/" << version << std::endl;
					}
#endif
				}
			}
		}
		// we gotta always have a runtime
		if (!foundRuntime)
		{
			std::pair<std::string,std::string> p("runtime",STRING(PRODUCT_VERSION)); //TODO: huh, what do we use?
			modules.push_back(std::pair< std::pair<std::string,std::string>, bool>(p,false));
		}
		file.close();
		return true;
	}
	std::string FileUtils::FindRuntime(int op, std::string& version)
	{
		std::string runtime = GetSystemRuntimeHomeDirectory();
		std::string path(runtime);
#ifdef OS_WIN32
		path += "\\runtime\\win32";
#elif OS_LINUX
		path += "/runtime/linux";
#elif OS_OSX
		path += "/runtime/osx";
#endif
		return FindVersioned(path,op,version);
	}
	std::string FileUtils::FindModule(std::string& name, int op, std::string& version)
	{
		std::string runtime = GetSystemRuntimeHomeDirectory();
		std::string path(runtime);
#ifdef OS_WIN32
		path += "\\modules\\win32\\";
#elif OS_LINUX
		path += "/modules/linux/";
#elif OS_OSX
		path += "/modules/osx/";
#endif
		path += name;
		return FindVersioned(path,op,version);
	}
	void FileUtils::ListDir(std::string& path, std::vector<std::string> &files)
	{
	#if defined(OS_WIN32)

		WIN32_FIND_DATA findFileData;
		std::string q(path+"\\*");
		HANDLE hFind = FindFirstFile(q.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				files.push_back(std::string(findFileData.cFileName));
			} while (FindNextFile(hFind, &findFileData));
			FindClose(hFind);
		}
	#else
		DIR *dp;
		struct dirent *dirp;
		if ((dp = opendir(path.c_str()))!=NULL)
		{
			while ((dirp = readdir(dp))!=NULL)
			{
				std::string fn = std::string(dirp->d_name);
				if (fn.substr(0,1)=="." || fn.substr(0,2)=="..")
				{
					continue;
				}
				files.push_back(fn);
			}
			closedir(dp);
		}
	#endif
	}
	int FileUtils::RunAndWait(std::string &path, std::vector<std::string> &args)
	{
#ifndef OS_WIN32
		std::string p;
		p+="\"";
		p+=path;
		p+="\" ";
		std::vector<std::string>::iterator i = args.begin();
		while (i!=args.end())
		{
			p+="\"";
			p+=(*i++);
			p+="\" ";
		}
#ifdef DEBUG
		std::cout << "running: " << p << std::endl;
#endif
		int status = system(p.c_str());
		return WEXITSTATUS(status);
#elif defined(OS_WIN32)
		std::ostringstream ostr;
		ostr << path.c_str();
		if (args.size() > 0 )
		{
			std::vector<std::string>::iterator i = args.begin();
			int idx = 0;
			while (i!=args.end())
			{
				// we need to quote each argument
				ostr << " \"" << (*i++).c_str() << "\"";
			}
		}
		DWORD rc=0;
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		char buf[MAX_PATH];
		DWORD size = GetCurrentDirectory(MAX_PATH,(char*)buf);
		buf[size]='\0';
		if (!CreateProcess( NULL,   // No module name (use command line)
							(char*)ostr.str().c_str(), // Command line
							NULL,           // Process handle not inheritable
							NULL,           // Thread handle not inheritable
							FALSE,          // Set handle inheritance to FALSE
							0,              // No creation flags
							NULL,           // Use parent's environment block
							(char*)buf,		// Use parent's starting directory
							&si,            // Pointer to STARTUPINFO structure
							&pi )           // Pointer to PROCESS_INFORMATION structure
		)
		{
			rc = -1;
		}
		else
		{
			// Wait until child process exits.
			WaitForSingleObject( pi.hProcess, INFINITE );

			// set the exit code
			GetExitCodeProcess(pi.hProcess,&rc);

			// Close process and thread handles.
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );
		}

		return rc;
#endif
	}

	#if defined(OS_WIN32)
	// TODO: implement this for other platforms
	void FileUtils::CopyRecursive(std::string &dir, std::string &dest)
	{
		if (!IsDirectory(dest)) {
			CreateDirectory(dest);
		}

		std::cout << "\n>Recursive copy " << dir << " to " << dest << std::endl;
		WIN32_FIND_DATA findFileData;
		std::string q(dir+"\\*");
		HANDLE hFind = FindFirstFile(q.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				std::string filename = findFileData.cFileName;
				if (filename == "." || filename == "..") continue;

				std::string srcName = dir + "\\" + filename;
				std::string destName = dest + "\\" + filename;

				if (IsDirectory(srcName)) {
					std::cout << "create dir: " << destName << std::endl;
					FileUtils::CreateDirectory(destName);
					CopyRecursive(srcName, destName);
				}
				else {
					//std::cout << "> copy file " << srcName << " to " << destName << std::endl;
					CopyFileA(srcName.c_str(), destName.c_str(), FALSE);
				}
			} while (FindNextFile(hFind, &findFileData));
			FindClose(hFind);
		}
	}

#endif


	std::string FileUtils::GetUsername()
	{
#ifdef OS_OSX
		return std::string([NSUserName() UTF8String]);
#elif OS_WIN32
		char buf[MAX_PATH];
		DWORD size = MAX_PATH;
        if (::GetUserName(buf,&size))
		{
			buf[size]='\0';
		}
		else
		{
			sprintf(buf,"Unknown");
		}
		return std::string(buf);
#elif OS_LINUX
		return std::string(getlogin());
#endif
	}
#ifndef NO_UNZIP
	void FileUtils::Unzip(std::string& source, std::string& destination)
	{
#ifdef OS_OSX
		//
		// we don't include gzip since we're on OSX
		// we just let the built-in OS handle extraction for us
		//
		std::vector<std::string> args;
		args.push_back("--noqtn");
		args.push_back("-x");
		args.push_back("-k");
		args.push_back("--rsrc");
		args.push_back(source);
		args.push_back(destination);
		std::string cmdline = "/usr/bin/ditto";
		RunAndWait(cmdline,args);
#elif OS_LINUX
		std::vector<std::string> args;
		args.push_back("-qq");
		args.push_back("-o");
		args.push_back(source);
		args.push_back("-d");
		args.push_back(destination);
		std::string cmdline("/usr/bin/unzip");
		RunAndWait(cmdline,args);
#elif OS_WIN32
		HZIP hz = OpenZip(source.c_str(),0);
		SetUnzipBaseDir(hz,destination.c_str());
		ZIPENTRY ze;
		GetZipItem(hz,-1,&ze);
		int numitems=ze.index;
		for (int zi=0; zi < numitems; zi++)
		{
			GetZipItem(hz,zi,&ze);
			UnzipItem(hz,zi,ze.name);
		}
		CloseZip(hz);
#endif
	}
#endif
}

