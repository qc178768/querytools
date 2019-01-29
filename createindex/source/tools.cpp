// tools.cpp
// 2003-11-03 Mike Liu: Add MkDir, PathExtractPath, PathExist, FileExist
// 2004-08-02 Mike Liu: There's a bug in __strrchrs() and PathExtractFile().
// 2006-08-09 Mike Liu: Fixed a potential bug in MyRTrim()
//#include "stdafx.h"
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#include <direct.h>
#endif
#include "tools.h"

inline bool __is_slash(char c)
{
	return ((c=='\\') || (c=='/'));
}

inline bool __is_absolute(const char* s)
{
	return (__is_slash(s[0]) ||
		( (strlen(s)>=3) && isalpha(s[0]) && s[1]==':' && __is_slash(s[2])) );
}

int PathAdd(char* szDest, int size, const char* szMore)
{
	if (!szMore) {
		return 0;
	}
	int iLenMore = (int)strlen(szMore);
	if (iLenMore==0) {
		return 0;
	}
	if (__is_absolute(szMore)) {
		if (iLenMore+1 > size) {
			return -1;
		}
		strcpy(szDest, szMore);
		szDest[iLenMore] = 0;
	} else {
		int iLenDest = (int)strlen(szDest);
		if (iLenDest>0) {
			if (__is_slash(szDest[iLenDest-1])) {
				if (iLenDest+iLenMore+1>size) {
					return -1;
				}
				strcat(szDest, szMore);
			} else {
				if (iLenDest+iLenMore+2>size) {
					return -1;
				}
				strcat(szDest, "/");
				strcat(szDest, szMore);
			}
		} else {
			if (iLenMore+1>size) {
				return -1;
			}
			strcpy(szDest, szMore);
		}
	}
	return 0;
}

int PathAdd(char* szDest, int size, const char* szSrc, const char* szMore)
{
	int iLenSrc = (int)strlen(szSrc);
	if (iLenSrc+1>size) {
		return -1;
	}
	strcpy(szDest, szSrc);
	return PathAdd(szDest, size, szMore);
}

inline char* __strrchrs(const char* s, const char* chrs)
{
	if (!chrs) {
		return NULL;
	}
	int len = (int)strlen(s);
	if (len==0) {
		return NULL;
	}
	for (const char* p = s+len-1; p>=s; p--) {
		if (strchr(chrs, *p)) {
			return (char*)p;
		}
	}
	return NULL;
/*
	if (!chrs) {
		return 0;
	}
	char* ret = 0;
	const char* p = chrs;
	while (*p) {
		if ((ret = (char*)strrchr(s, *p))!=0) {
			break;
		}
		p++;
	}
	return ret;
*/
}

int PathExtractFile(char* szFile, int size, const char* szPathFile)
{
	memset(szFile, 0, size);
	if (!szPathFile) {
		return -1;
	}
	int iLenPathFile = (int)strlen(szPathFile);
	if (iLenPathFile==0) {
		return -1;
	}
	char* szTemp = __strrchrs(szPathFile, ":\\/");
	if (szTemp) {	// Found the path splitter
		szTemp++;
		if (strlen(szTemp)+1 > (unsigned int)size) {
			return -1;
		}
		strncpy(szFile, szTemp, size-1);
	} else {		// Only file name, no path
		if (iLenPathFile+1 > size) {
			return -1;
		}
		strncpy(szFile, szPathFile, size-1);
	}
	if (strlen(szFile)==0) {
		return -1;
	}
	return 0;
}

int PathExtractPath(char* szPath, int size, const char* szPathFile)
{
	memset(szPath, 0, size);
	if (!szPathFile) {
		return -1;
	}
	int iLenPathFile = (int)strlen(szPathFile);
	if (iLenPathFile==0) {
		return -1;
	}
	char* szTemp = __strrchrs(szPathFile, ":\\/");
	if (szTemp) {	// Found the path splitter
		int iLenPath = int(szTemp-szPathFile);
		if (iLenPath+1>size) {
			iLenPath = size-1;
		}
		strncpy(szPath, szPathFile, iLenPath);
	}
	if (strlen(szPath)==0) {
		return -1;
	}
	return 0;
}

bool PathExist(const char* path)
{
#ifdef WIN32
	struct _stat st;
	if (_stat(path, &st)!=0) {
		return false;
	}
	if (st.st_mode & _S_IFDIR) {
		return true;
	} else {
		return false;
	}
#else
	struct stat st;
	if (stat(path, &st)!=0) {
		return false;
	}
	if (st.st_mode & S_IFDIR) {
		return true;
	} else {
		return false;
	}
#endif
}

bool FileExist(const char* file)
{
#ifdef WIN32
	struct _stat st;
	if (_stat(file, &st)!=0) {
		return false;
	}
	if (st.st_mode & _S_IFREG) {
		return true;
	} else {
		return false;
	}
#else
	struct stat st;
	if (stat(file, &st)!=0) {
		return false;
	}
	if (st.st_mode & S_IFREG) {
		return true;
	} else {
		return false;
	}
#endif
}

int MkDir(const char* dir)
{
	if (PathExist(dir)) {
		return 0;
	}

	int ret = 0;
	char cParent[256];
	if (PathExtractPath(cParent, (int)sizeof(cParent), dir)==0) {
		if (!PathExist(cParent)) {
			if ((ret = MkDir(cParent))!=0) {
				return ret;
			}
		}
	}


	printf("mkdir %s\n", dir);
#ifdef WIN32
	ret = _mkdir(dir);
#else
	//return mkdir(dir, S_ISUID|S_ISGID|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	ret = mkdir(dir, 0777);
#endif
	if (ret!=0) {
		if (errno==EEXIST) {
			return 0;
		}
	}
	return 0;
}

time_t CurrDate()
{
	time_t now = time(0);
	struct tm res;
	memset(&res, 0, sizeof(res));
	struct tm* tmp = localtime_r(&now, &res);
	tmp->tm_hour = 0;
	tmp->tm_min = 0;
	tmp->tm_sec = 0;
	return mktime(tmp);
}

int StrToSeconds(const char* szTime)
{
	char buff[32];
	memset(buff, 0, sizeof(buff));
	strncpy(buff, szTime, sizeof(buff)-1);

	int ret = 0;
	char* lasts = 0;
	char* hour = strtok_r(buff, ":# ", &lasts);
	if (hour) {
		ret += atoi(hour)*60*60;
		char* min = strtok_r(NULL, ":# ", &lasts);
		if (min) {
			ret += atoi(min)*60;
		}
	}
	return ret;
}

time_t StrToDateTime(const char* szDateTime)
{
	if (!szDateTime) {
		return -1;
	}
	if (strlen(szDateTime)<DATETIMESIZE) {
		return -1;
	}

	char year[YEARSIZE+1];
	char month[MONTHSIZE+1];
	char day[DAYSIZE+1];
	char hour[HOURSIZE+1];
	char min[MINUTESIZE+1];
	char sec[SECONDSIZE+1];
	memset(year, 0, sizeof(year));
	memset(month, 0, sizeof(month));
	memset(day, 0, sizeof(day));
	memset(hour, 0, sizeof(hour));
	memset(min, 0, sizeof(min));
	memset(sec, 0, sizeof(sec));

	int offset = 0;
	memcpy(year, szDateTime, sizeof(year)-1);
	offset += int(sizeof(year)-1);
	memcpy(month, szDateTime+offset, sizeof(month)-1);
	offset += int(sizeof(month)-1);
	memcpy(day, szDateTime+offset, sizeof(day)-1);
	offset += int(sizeof(day)-1);
	memcpy(hour, szDateTime+offset, sizeof(hour)-1);
	offset += int(sizeof(hour)-1);
	memcpy(min, szDateTime+offset, sizeof(min)-1);
	offset += int(sizeof(min)-1);
	memcpy(sec, szDateTime+offset, sizeof(sec)-1);

	struct tm res;
	memset(&res, 0, sizeof(res));
	res.tm_year = atoi(year) - 1900;
	res.tm_mon = atoi(month) - 1;
	res.tm_mday = atoi(day);
	res.tm_hour = atoi(hour);
	res.tm_min = atoi(min);
	res.tm_sec = atoi(sec);

	return mktime(&res);
}

time_t StrToDate(const char* szDate)
{
	if (!szDate) {
		return -1;
	}
	if (strlen(szDate)<DATESIZE) {
		return -1;
	}

	char year[YEARSIZE+1];
	char month[MONTHSIZE+1];
	char day[DAYSIZE+1];
	memset(year, 0, sizeof(year));
	memset(month, 0, sizeof(month));
	memset(day, 0, sizeof(day));

	int offset = 0;
	memcpy(year, szDate, sizeof(year)-1);
	offset += int(sizeof(year)-1);
	memcpy(month, szDate+offset, sizeof(month)-1);
	offset += int(sizeof(month)-1);
	memcpy(day, szDate+offset, sizeof(day)-1);

	struct tm res;
	memset(&res, 0, sizeof(res));
	res.tm_year = atoi(year) - 1900;
	res.tm_mon = atoi(month) - 1;
	res.tm_mday = atoi(day);

	return mktime(&res);
}

char* DateTimeToStr(time_t t, char* buff, int size)
{
	if (!buff || size<=DATETIMESIZE) {
		return 0;
	}

	struct tm res;
	memset(&res, 0, sizeof(res));
	strftime(buff, size, "%Y%m%d%H%M%S", localtime_r(&t, &res));
	return buff;
}

char* DateToStr(time_t t, char* buff, int size)
{
	if (!buff || size<=DATESIZE) {
		return 0;
	}

	struct tm res;
	memset(&res, 0, sizeof(res));
	strftime(buff, size, "%Y%m%d", localtime_r(&t, &res));
	return buff;
}

char* TimeToStr(time_t t, char* buff, int size)
{
	if (!buff || size<=TIMESIZE) {
		return 0;
	}

	struct tm res;
	memset(&res, 0, sizeof(res));
	strftime(buff, size, "%H%M%S", localtime_r(&t, &res));
	return buff;
}

char* StrLPad(char* s, int maxsize, const char* pad)
{
	int padsize = (int)strlen(pad);
	int ssize = (int)strlen(s);
	if (padsize+ssize+1>maxsize) {
		return 0;
	}
	memmove(s+padsize, s, ssize+1);
	memcpy(s, pad, padsize);
	return s;
}

char* StrLCut(char* s, int cutsize)
{
	int len = (int)strlen(s);
	if (len<=cutsize) {
		*s = 0;
	} else {
		while (*(s+cutsize)) {
			*s = *(s+cutsize);
			s++;
		}
		*s = *(s+cutsize);
	}
	return s;
}

char* SubStr(const char* src, int start, int len, char* substr)
{
	register int i = 0;
	for (i = 0; i<len && *(src+start+i); i++) {
		*(substr+i) = *(src+start+i);
	}
	*(substr+i) = 0;
	return substr;
}


bool IsMyTrimChar(char c)
{
	return c==' '||c=='\t'||c=='\n'||c=='\r';
}

char* MyLTrim(char* buff)
{
	if (!buff) {
		return NULL;
	}
	register char* p = buff;
	while (IsMyTrimChar(*p)) {
		p++;
	}
	return p;
}

char* MyRTrim(char* buff)
{
	if (buff!=NULL) {
		register size_t len = strlen(buff);
		if (len>0) {
			char* p = buff+len-1;
			while (p>=buff && IsMyTrimChar(*p)) {
				*p = 0;
				p--;
			}
		}
	}
	return buff;
}

char* MyTrim(char* buff)
{
	return MyRTrim(MyLTrim(buff));
}


#ifdef NETSUPPORT

///////////////////////////////////////////////////////////////
// Socket functions

#ifndef INADDR_NONE
#define INADDR_NONE		0xffffffff
#endif

#ifndef WIN32
inline int closesocket(int s)
{
	return close(s);
}
#endif


int ListenTCP(const char* port, int qlen)
{
	int s = -1;						// Socket descriptor
	int	type = 0;					// Socket type
	struct servent	*pse = 0;		// Pointer to service information entry
	struct protoent *ppe = 0;		// Pointer to protocol information entry
	struct sockaddr_in sin;			// an Internet endpoint address
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	// Map service name to port number
	if (pse = getservbyname(port, "tcp")) {
		sin.sin_port = htons(ntohs((u_short)pse->s_port));
	} else if ((sin.sin_port = htons((u_short)atoi(port)))==0) {
		return -1;
	}

	/////////////////////////////////////////////////////
	// Map protocol name to protocol number
	if ((ppe = getprotobyname("tcp"))==0) {
		return -1;
	}

	// Use protocol to choose a socket type
	type = SOCK_STREAM;

	/////////////////////////////////////////////////////
	// Allocate a socket
	if ((s = socket(PF_INET, type, ppe->p_proto))<0) {
		printf("SOCK: listen failed: allocate socket failed - code (%d) msg (%s)\n", errno, strerror(errno));
		return -1;
	}

	struct linger lin;
	lin.l_onoff = 0;
	lin.l_linger = 0;
	setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*)&lin, sizeof(lin));
	int reuse = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	/////////////////////////////////////////////////////
	// Bind the socket
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin))<0) {
		printf("SOCK: listen failed: bind socket failed - code (%d) msg (%s)\n", errno, strerror(errno));
		closesocket(s);
		return -1;
	}

	if (type == SOCK_STREAM && listen(s, qlen)<0) {
		printf("SOCK: listen failed: listen failed - code (%d) msg (%s)\n", errno, strerror(errno));
		CloseSocket(s);
		return -1;
	}

	return s;
}

int ConnectTCP(const char* host, const char* port)
{
	struct hostent*		phe = 0;	// pointer to host information entry
	struct servent*		pse = 0;	// pointer to service information entry
	struct protoent*	ppe = 0;	// pointer to protocol information entry
	struct sockaddr_in	sin;	// an Internet endpoint address
	int s = -1;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

	// Map service name to port number
	if (pse = getservbyname(port, "tcp")) {
		sin.sin_port = pse->s_port;
	} else if ((sin.sin_port = htons((u_short)atoi(port)))==0) {
		printf("SOCK: connect failed: bad port '%s'!\n", port);
		return -1;
	}

	// Map host name to IP address, allowing for dotted decimal
	if (phe = gethostbyname(host)) {
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);	// here only get the first IP address
	} else if ((sin.sin_addr.s_addr = inet_addr(host))==INADDR_NONE) {
		printf("SOCK: connect failed: bad host '%s'!\n", host);
		return -1;
	}

	// Map transport protocol name to protocol number
	if ((ppe = getprotobyname("tcp"))==0) {
		return -1;
	}

	// Allocate a socket
	if ((s = socket(PF_INET, SOCK_STREAM, ppe->p_proto))<0) {
		printf("SOCK: connect failed: allocate socket failed - code (%d) msg (%s)\n", errno, strerror(errno));
		return -1;
	}

	if (connect(s, (struct sockaddr*)&sin, sizeof(sin))<0) {
		// printf("SOCK: connect failed: host:port is '%s : %s' - code (%d) msg (%s)!\n", host, port, errno, strerror(errno));
		CloseSocket(s);
		return -1;
	}

	if (s<0) {
		return -1;
	}

	return s;
}

int CloseSocket(int s)
{
	shutdown(s, SHUT_RDWR);
	return closesocket(s);
}

int Recv(int s, char* buff, int len, int flags)
{
	int remain = len;
	while (remain>0) {
		int ret = recv(s, buff+len-remain, remain, flags);
		if (ret==0) {
			printf("SOCK: recv failed: connection closed by remote.\n");
			return ret;		// Closed by remote
		} else if (ret<0) {
			if (errno==EINTR) {
				printf("SOCK: recv interrupted, retrying...\n");
				continue;
			}
			printf("SOCK: recv failed: error occurs - code (%d), msg (%s)\n", errno, strerror(errno));
			return ret;
		}
		remain -= ret;
	}
	return len;
}

int Send(int s, const char* buff, int len, int flags)
{
	int remain = len;
	while (remain>0) {
		int ret = send(s, buff, remain+len-remain, flags);
		if (ret==0) {
			printf("SOCK: send failed: connection closed by remote.\n");
			return ret;
		} else if (ret<0) {
			if (errno==EINTR) {
				printf("SOCK: send interrupted, retrying...\n");
				continue;
			}
			printf("SOCK: send failed: error occurs - code (%d), msg (%s)\n", errno, strerror(errno));
			return ret;
		}
		remain -= ret;
	}
	return len;
}

int WaitToRecv(int s, int sec, int usec)
{
	int ret = -1;
	if (s<0) {
		return ret;
	}

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = usec;
	if ((ret = select(s+1, &fds, 0, 0, &tv))<=0) {
		return ret;
	}
	if (!FD_ISSET(s, &fds)) {
		return -1;
	}
	return ret;
}

int WaitToSend(int s, int sec, int usec)
{
	int ret = -1;
	if (s<0) {
		return ret;
	}

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(s, &fds);
	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = usec;
	if ((ret = select(s+1, 0, &fds, 0, &tv))<=0) {
		return ret;
	}
	if (!FD_ISSET(s, &fds)) {
		return -1;
	}
	return ret;
}

#endif



int hexprintf(const char* data, int size, FILE* fp)
{
	if (!fp)
		return 1;

	if (size<=0)
		return 2;

	static char dict[] = "0123456789ABCDEF";

	char hex[50];
	char printable[17];
	hex[sizeof(hex)-1] = 0;
	printable[sizeof(printable)-1] = 0;

	int lineno = 0;
	int i = 0;
	while (true)
	{
		if (i%16==0 || i==size)
		{
			if (i!=0 || i==size)
			{
				fprintf(fp, "%08x  %s %s\n", lineno*16, hex, printable);
				lineno++;
				if (i==size)
					break;
			}

			int j = 0;
			for (j = 0; j<sizeof(hex)-1; j++)
			{
				*(hex+j) = ' ';
			}
			for (j = 0; j<sizeof(printable)-1; j++)
			{
				*(printable+j) = 0;
			}
		}

		int hexindex = (i%16>=8) ? ((i%16)*3+1) : ((i%16)*3);
		unsigned char c = (unsigned char)data[i];
		unsigned char hi = (c&0xf0)>>4;
		unsigned char lo = (c&0x0f);
		hex[hexindex] = dict[hi];
		hex[hexindex+1] = dict[lo];

		if (data[i]>0&&isprint(data[i])&&(data[i]!='\r')&&(data[i]!='\n')&&(data[i]!='\t'))
		{
			printable[i%16] = data[i];
		}
		else
		{
			printable[i%16] = '.';

		}
		i++;
	}

	return 0;
}

void indent_print(FILE* fp, int i)
{
	fprintf(fp, "%s", indent(i));
}

const char* indent(int i)
{
	static const char* blanks = "                                ";
	static const size_t len = strlen(blanks);

	i<<=1;
	return (blanks+(((size_t)i<len) ? (len-((size_t)i)) : 0));
}
