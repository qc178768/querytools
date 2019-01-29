// string2.h
//
// History:
// 2004-01-19 Mike Liu	Add buffsize()
//	2004-01-30 Mike Liu	Add operator +=
//	2004-08-16 Mike Liu Fixed a bug - while resize, we'll copy
//						the old data.
//	2004-09-06 Mike Liu Increase performance.
//	2004-09-07 Mike Liu Use malloc/realloc/free to increase perf
//	2005-02-02 Mike Liu Remove compile warning on bits 64
//------------------------------------------------------------------

#ifndef __string2_h__
#define __string2_h__

#include <stdio.h>
//#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string>

using namespace std;

#define STR2_ALIGNBITS		0x1f
#define STR2_ALIGN(n)		(((n) & (~(STR2_ALIGNBITS)))+(STR2_ALIGNBITS)+1)

class string2
{
public:
	typedef char	value_type;
	typedef size_t	size_type;
	enum {
					npos = -1
	};

private:
	char		m_buff[256];
	char*		m_dyna;
	size_type	m_size;
	size_type	m_buffsize;

protected:

	#define is_dyna_buff(dyna) ((dyna)!=m_buff)
	#define is_dyna(dyna, ssize) (is_dyna_buff(dyna) || ((ssize)>=sizeof(m_buff)))

	// Reallocate memory if nessisary.
	bool dyna_realloc(size_type len) {
		if (len>=m_buffsize) {
			char* dyna = is_dyna_buff(m_dyna) ? m_dyna : NULL;
			if (NULL!=(m_dyna = (char*)::realloc(dyna, m_buffsize = STR2_ALIGN(len+1)))) {
				m_dyna[m_size] = 0;
			} else {
				m_size = 0;
				return false;
			}
			if (!dyna) {	// Switch to dynamic buffer
				memmove(m_dyna, m_buff, m_size);
				m_buff[0] = 0;
			}
		}
		return true;
	}


public:
	string2():
		m_size(0),
		m_buffsize(0)
	{
		m_buff[0] = 0;
		m_dyna = m_buff;
	}

	string2(const string2& other):
		m_size(0),
		m_buffsize(0)
	{
		m_buff[0] = 0;
		m_dyna = m_buff;
		*this = other;
	}

	string2(const char* s):
		m_size(0),
		m_buffsize(0)
	{
		m_buff[0] = 0;
		m_dyna = m_buff;
		*this = s;
	}

	string2(const char* s, size_type ssize):
		m_size(0),
		m_buffsize(0)
	{
		m_buff[0] = 0;
		m_dyna = m_buff;
		assign(s, ssize);
	}

	string2(const std::string& s):
		m_size(0),
		m_buffsize(0)
	{
		m_buff[0] = 0;
		m_dyna = m_buff;
		*this = s;
	}

	string2(char c):
		m_size(0),
		m_buffsize(0)
	{
		m_buff[0] = 0;
		m_dyna = m_buff;
		*this = c;
	}

	virtual ~string2()
	{
		if (is_dyna_buff(m_dyna)) {
			::free(m_dyna);
		}
	}

	void reserve(size_type ssize)
	{
		if (is_dyna(m_dyna, ssize)) {
			dyna_realloc(ssize);
		}
	}

	const char* c_str() const { return m_dyna; }
	const char* data() const { return m_dyna; }
	char* data()   { return m_dyna; }
	size_type size() const { return m_size; }
	size_type buffsize() const { return is_dyna_buff(m_dyna) ? m_buffsize : sizeof(m_buff); }
	operator const char*() const { return m_dyna; }
	operator char*() { return m_dyna; }
	bool empty() const { return size()==0; }
	void clear()
	{
		*m_dyna = 0;
		m_size = 0;
	}

	void rtrim(char c = ' ')
	{
		if (m_size>0) {
			register char* buff = data();
			register long int i = m_size-1;
			for (; i>=0; i--) {
				if (buff[i]==c) {
					buff[i] = 0;
				} else {
					break;
				}
			}
			m_size = i+1;
		}
	}

	void ltrim(char c = ' ')
	{
		register char* buff = data();
		register int i = 0;
		for (; i<m_size; i++) {
			if (buff[i]!=c) {
				break;
			}
		}
		if (i!=0) {
			memmove(buff, buff+i, m_size-i+1);
			m_size -= i;
		}
	}

	string2& operator =(const char* s)
	{
		s = s ? s : "";
		assign(s, strlen(s));
		return *this;
	}

	string2& operator =(const string2& other)
	{
		assign(other.c_str(), other.size());
		return *this;
	}

	string2& operator =(const std::string& s)
	{
		assign(s.c_str(), s.size());
		return *this;
	}

	string2& operator =(char c)
	{
		assign(&c, 1);
		return *this;
	}

	string2& operator += (char c)
	{
		append(&c, sizeof(c));
		return *this;
	}

	string2& operator += (int c)
	{
		return operator +=(char(c));
	}

	string2& operator += (const char* s)
	{
		s = s ? s : "";
		append(s, strlen(s));
		return *this;
	}

	string2& operator += (const string2& other)
	{
		append(other.c_str(), other.size());
		return *this;
	}

	string2& operator += (const std::string& s)
	{
		append(s.c_str(), s.size());
		return *this;
	}

	size_type find(char c, size_type pos = 0) const {
		char* p = strchr(m_dyna+pos, c);
		if (p) {
			return (size_type)(p-m_dyna);
		} else {
			return (size_type)npos;
		}
	}

	size_type find(const char* s, size_type pos = 0) const {
		char* p = strstr(m_dyna+pos, s);
		if (p) {
			return (size_type)(p-m_dyna);
		} else {
			return (size_type)npos;
		}
	}

	char& operator [](size_type i) { return m_dyna[i]; }

	const char& operator [](size_type i) const { return m_dyna[i]; }

	string2 substr(size_type pos, size_type len = npos) const {
		if (pos>m_size) {
			return string2("");
		} else {
			register size_type remain = (m_size-pos);
			if (len==npos || len>remain) {
				len = remain;
			}
			return string2(m_dyna+pos, len);
		}
	}


	string2& assign(const char* s, size_type ssize) {
		if (is_dyna(m_dyna, ssize) && ssize>=m_buffsize) {
			if (is_dyna_buff(m_dyna)) {
				::free(m_dyna);
			}
			if (NULL==(m_dyna = (char*)::malloc(m_buffsize = STR2_ALIGN(ssize+1)))) {
				m_size = 0;
				return *this;
			}
		}
		memmove(m_dyna, s, ssize);
		m_dyna[ssize] = 0;
		m_size = ssize;
		return *this;
	}


	string2& append(const char* s, size_type ssize) {
		register size_type size2 = m_size+ssize;
		if (is_dyna(m_dyna, size2)) {
			if (!dyna_realloc(size2)) {
				return *this;
			}
		}
		memmove(m_dyna+m_size, s, ssize);
		m_dyna[size2] = 0;
		m_size = size2;
		return *this;
	}
};


typedef string2 String;

// Operator ==
inline bool operator ==(const string2& s, const char* p) {
	return strcmp(s.c_str(), p)==0;
}

inline bool operator ==(const string2& s, const std::string& ss) {
	return strcmp(s.c_str(), ss.c_str())==0;
}

inline bool operator ==(const char* p, const string2& s) {
	return strcmp(p, s.c_str())==0;
}

inline bool operator ==(const std::string ss, const string2& s) {
	return strcmp(ss.c_str(), s.c_str())==0;
}

inline bool operator ==(const string2& s1, const string2& s2) {
	return strcmp(s1.c_str(), s2.c_str())==0;
}


// Operator !=
inline bool operator !=(const string2& s, const char* p) {
	return strcmp(s.c_str(), p)!=0;
}

inline bool operator !=(const string2& s, const std::string& ss) {
	return strcmp(s.c_str(), ss.c_str())!=0;
}

inline bool operator !=(const char* p, const string2& s) {
	return strcmp(p, s.c_str())!=0;
}

inline bool operator !=(const std::string ss, const string2& s) {
	return strcmp(ss.c_str(), s.c_str())!=0;
}

inline bool operator !=(const string2& s1, const string2& s2) {
	return strcmp(s1.c_str(), s2.c_str())!=0;
}


// Operator <
inline bool operator <(const string2& s, const char* p) {
	return strcmp(s.c_str(), p)<0;
}

inline bool operator <(const string2& s, const std::string& ss) {
	return strcmp(s.c_str(), ss.c_str())<0;
}

inline bool operator <(const char* p, const string2& s) {
	return strcmp(p, s.c_str())<0;
}

inline bool operator <(const std::string ss, const string2& s) {
	return strcmp(ss.c_str(), s.c_str())<0;
}

inline bool operator <(const string2& s1, const string2& s2) {
	return strcmp(s1.c_str(), s2.c_str())<0;
}


// Operator <=
inline bool operator <=(const string2& s, const char* p) {
	return strcmp(s.c_str(), p)<=0;
}

inline bool operator <=(const string2& s, const std::string& ss) {
	return strcmp(s.c_str(), ss.c_str())<=0;
}

inline bool operator <=(const char* p, const string2& s) {
	return strcmp(p, s.c_str())<=0;
}

inline bool operator <=(const std::string ss, const string2& s) {
	return strcmp(ss.c_str(), s.c_str())<=0;
}

inline bool operator <=(const string2& s1, const string2& s2) {
	return strcmp(s1.c_str(), s2.c_str())<=0;
}


// Operator >
inline bool operator >(const string2& s, const char* p) {
	return strcmp(s.c_str(), p)>0;
}

inline bool operator >(const string2& s, const std::string& ss) {
	return strcmp(s.c_str(), ss.c_str())>0;
}

inline bool operator >(const char* p, const string2& s) {
	return strcmp(p, s.c_str())>0;
}

inline bool operator >(const std::string ss, const string2& s) {
	return strcmp(ss.c_str(), s.c_str())>0;
}

inline bool operator >(const string2& s1, const string2& s2) {
	return strcmp(s1.c_str(), s2.c_str())>0;
}


// Operator >=
inline bool operator >=(const string2& s, const char* p) {
	return strcmp(s.c_str(), p)>=0;
}

inline bool operator >=(const string2& s, const std::string& ss) {
	return strcmp(s.c_str(), ss.c_str())>=0;
}

inline bool operator >=(const char* p, const string2& s) {
	return strcmp(p, s.c_str())>=0;
}

inline bool operator >=(const std::string ss, const string2& s) {
	return strcmp(ss.c_str(), s.c_str())>=0;
}

inline bool operator >=(const string2& s1, const string2& s2) {
	return strcmp(s1.c_str(), s2.c_str())>=0;
}

#endif

