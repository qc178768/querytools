// tools.h
// History:
// When       Who            What
// 2003-07-10 Mike Liu	Add StrLPad function
// 2003-11-14 Mike Liu	Add macro chararrayzero, chararraycpy and dim.
// 2003-12-26 Mike Liu	Add CTimeVal.
// 2003-12-30 Mike Liu	Purify some code for CTimeVal, add NETSUPPORT
//								macro.
// 2004-03-03 Mike Liu	Add mytrim() functions.
// 2004-08-11 Mike Liu	Add indent_print() function.
//-------------------------------------------------------------------

#ifndef __tools_h__
#define __tools_h__

// If you don't need net support, just
// comment following line.
// #define NETSUPPORT

#ifdef WIN32
#ifdef NETSUPPORT
#include <winsock2.h>
#endif
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <memory.h>
#include <errno.h>

#ifdef WIN32

#ifndef localtime_r
#define localtime_r(t, res) localtime(t)
#endif

#ifndef strtok_r
#define strtok_r(s, ss, lasts) strtok(s, ss)
#endif

#endif	// WIN32


#define const_strlen(x) (sizeof(x)-1)


#define YEARSIZE		(sizeof("yyyy")-1)
#define MONTHSIZE		(sizeof("mm")-1)
#define DAYSIZE			(sizeof("dd")-1)
#define HOURSIZE		(sizeof("HH")-1)
#define MINUTESIZE		(sizeof("MM")-1)
#define SECONDSIZE		(sizeof("SS")-1)
#define DATESIZE		(YEARSIZE+MONTHSIZE+DAYSIZE)
#define TIMESIZE			(HOURSIZE+MINUTESIZE+SECONDSIZE)
#define DATETIMESIZE	(DATESIZE+TIMESIZE)

inline char* sncpy(char* s, const char* source, size_t size) {
	if (!size) {
		return s;
	}
	char* end = s+size-1;
	register char* q = s;
	register const char* p = source;
	if (p) {
		while (*p && q<end) {
			*q++ = *p++;
		}
	}
	*q = 0;
	return s;
}

//----------------------------------------------
// Huge integer support
//----------------------------------------------

#ifdef WIN32
#define HUGEINT	__int64
#else
#define HUGEINT long long
#endif

#ifdef WIN32
inline HUGEINT atoll(const char* s) {
	return _atoi64(s);
}
#endif
#ifdef HPUX
inline HUGEINT atoll(const char* s) {
	register char* last = NULL;
	return strtoll(s, &last, 10);
}

#endif

inline char* lltoa(const HUGEINT& ll, char* buff) {
#ifdef WIN32
	sprintf_s(buff, 10, "%I64d", ll);
	return buff;
#else
	sprintf(buff, "%lld", ll);
	return buff;
#endif
}


//----------------------------------------------
// Cahr array operators
//----------------------------------------------
#ifndef chararrayzero
#define chararrayzero(s) memset(s, 0, sizeof(s))
#define chararraycpy(s, src) sncpy(s, src, sizeof(s))
#endif

// Use this function to get the elements count
// of a C language array.
#ifndef dim
#define dim(x) (sizeof(x)/sizeof(*x))
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

int PathAdd(char* szDest, int size, const char* szMore);
int PathAdd(char* szDest, int size, const char* szSrc, const char* szMore);
int PathExtractFile(char* szFile, int size, const char* szPathFile);
int PathExtractPath(char* szPath, int size, const char* szPathFile);
bool PathExist(const char* path);
bool FileExist(const char* file);
int MkDir(const char* dir);
char* StrLPad(char* s, int maxsize, const char* pad);
char* StrLCut(char* s, int cutsize);
char* SubStr(const char* src, int start, int len, char* substr);
time_t CurrDate();						// Return current date, with HH:MM:SS = 00:00:00
time_t StrToDate(const char* szDate);	// Convert 'yyyymmdd' to time_t
time_t StrToDateTime(const char* szDateTime); // Convert 'yyyymmddHHMMSS' to time_t
char* DateTimeToStr(time_t t, char* buff, int size);
char* DateToStr(time_t t, char* buff, int size);	// "yyyymmdd"
char* TimeToStr(time_t t, char* buff, int size);	// "HHMMSS"
int StrToSeconds(const char* szTime);	// Return seconds offset of MM:SS

// MyTrim functions
bool IsMyTrimChar(char c);
char* MyLTrim(char* buff);
char* MyRTrim(char* buff);
char* MyTrim(char* buff);


#ifdef NETSUPPORT
int ListenTCP(const char* port, int qlen);
int ConnectTCP(const char* host, const char* port);
int CloseSocket(int s);
int Recv(int s, char* buff, int len, int flags);
int Send(int s, const char* buff, int len, int flags);
int WaitToRecv(int s, int sec, int usec);
int WaitToSend(int s, int sec, int usec);
#endif

#ifndef WIN32

struct CTimeVal:
	public timeval
{
public:
	CTimeVal()
	{
		tv_sec = 0;
		tv_usec = 0;
		setnow();
	}

	CTimeVal(const struct timeval& tv)
	{
		tv_sec = tv.tv_sec;
		tv_usec = tv.tv_usec;
	}

	CTimeVal(const CTimeVal& other)
	{
		tv_sec = 0;
		tv_usec = 0;
		*this = other;
	}

	CTimeVal& operator =(const CTimeVal& other)
	{
		if (this!=&other) {
			tv_sec = other.tv_sec;
			tv_usec = other.tv_usec;
		}
		return *this;
	}

	// Set this object as current time.
	int setnow()
	{
		int ret = 0;
		if ((ret = gettimeofday(this, NULL))!=0) {
			printf("gettimeofday() failed (ret=%d): %s!\n", ret, strerror(errno));
		}
		return ret;
	}

	// This time - other, return the ms.
	int diffms(const CTimeVal& other) const
	{
		return int((tv_sec-other.tv_sec)*1000+(tv_usec-other.tv_usec)/1000);
	}


	// Increase time as ms.
	int addms(int ms)
	{
		tv_sec += ms/1000;
		tv_usec += (ms%1000)*1000;
		tv_sec += tv_usec/1000000;
		tv_usec = tv_usec%1000000;
		return 0;
	}

	// Set time as ms.
	int setms(int ms)
	{
		tv_sec = ms/1000;
		tv_usec = (ms%1000)*1000;
		return 0;
	}

	int elapsems() const
	{
		CTimeVal now;
		return now.diffms(*this);
	}
};

#endif

int hexprintf(const char* data, int size, FILE* fp = stdout);
void indent_print(FILE* fp = stdout, int indent = 0);
const char* indent(int i);

inline char* mysncpy(char* s, const char* source, size_t size) {
	if (!size) {
		return s;
	}
	char* end = s+size-1;
	register char* q = s;
	register const char* p = source;
	while (*p && q<end) {
		*q++ = *p++;
	}
	*q = 0;
	return s;
}

inline char* myitostr(char* s, int ivalue) {
	char *p = s;                // pointer to traverse string
	if (ivalue<0) {
		// negative, so output '-' and negate
		*p++ = '-';
		ivalue = (unsigned int)(-(int)ivalue);
	}

	char* firstdig = p;		// save pointer to first digit
	unsigned digval;		// value of digit
	do {
		digval = (unsigned) (ivalue % 10);
		ivalue /= 10;       // get next digit

		// convert to ascii and store
		*p++ = (char)(digval+'0');
	} while (ivalue > 0);

	// We now have the digit of the number in the buffer, but in reverse
	// order.  Thus we reverse them now.

	*p-- = '\0';            // terminate string; p points to last digit

	char temp;              // temp char
	do {
		temp = *p;
		*p = *firstdig;
		*firstdig = temp;	// swap *p and *firstdig
		--p;
		++firstdig;			// advance to next two digits
	} while (firstdig < p);	// repeat until halfway
	return s;
}


inline char* myutostr(char* s, unsigned int ivalue) {
	char *p = s;                // pointer to traverse string
	char* firstdig = p;		// save pointer to first digit
	unsigned digval;		// value of digit
	do {
		digval = (unsigned) (ivalue % 10);
		ivalue /= 10;       // get next digit

		// convert to ascii and store
		*p++ = (char)(digval+'0');
	} while (ivalue > 0);

	// We now have the digit of the number in the buffer, but in reverse
	// order.  Thus we reverse them now.

	*p-- = '\0';            // terminate string; p points to last digit

	char temp;              // temp char
	do {
		temp = *p;
		*p = *firstdig;
		*firstdig = temp;	// swap *p and *firstdig
		--p;
		++firstdig;			// advance to next two digits
	} while (firstdig < p);	// repeat until halfway
	return s;
}

#endif

