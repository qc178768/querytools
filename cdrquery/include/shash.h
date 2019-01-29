// shash.h
//
// Version		Date			Author		Description
// 0001.02		2003-12-17	Mike Liu	Add sohash template.
// 0001.03		2004-01-15	Mike Liu	Use array to access bucket
//												Add size().
// 0001.04		2004-01-19	Mike Liu	Use T() to init default.
//												Use string2
// 0001.05		2004-01-29	Mike Liu	Use initialize table.
// 0001.06		2004-05-20	Mike Liu	Use memset to init to inc perf.
// 0001.07		2004-05-21	Mike Liu	Fixed a bug in hash function.
// 0001.08		2004-05-24	Mike Liu	Use shashfun().
// 0001.09		2004-07-14	Mike Liu	Chang member max() to max_buck to avoid VC compile error!
// 0001.10		2004-09-07	Mike Liu	Avoidance the SOPair
//------------------------------------------------------------------

#ifndef __shash_h__
#define __shash_h__

#include <stdio.h>
#include <stdlib.h>
//#include <stddef.h>
#include <string.h>
#include <memory.h>
#ifndef WIN32
#include <strings.h>
#endif
#include "string2.h"
#include "objalloc.h"

#ifdef WIN32
#define bzero(s, size) memset(s, 0, size)
#endif

#define SHASH_PRIME				2049982463
#define SHASH_MAX_NAME			128
#define SHASH_MAX_VALUE			256
#define SHASH_DEFAULT_BUCK		251


// Function:
//		Copy upto size count of char from source to s.
inline char* mystrncpy(char* s, const char* source, size_t size) {
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

inline unsigned long shashfun(const char* s) {
	if (!s) {
		return 0;
	}
	register unsigned long rtn = 0;
	for (register const char* p = s; *p; p++) {
		rtn = (rtn<<5)+*p;
	}
	return rtn;
}

inline unsigned long shashfun1(const char* s) {
	if (!s) {
		return 0;
	}
	register unsigned long rtn = 0;
	register unsigned long n = 0;
	register unsigned long i = 0;
	register const char* p = s;
	while (*p) {
		for (i = 0; i<sizeof(n); i++) {
			if (*p==0) {
				return (rtn += n);
			}
			n = (n<<8)+(unsigned char)(*p);
			p++;
		}
		rtn += n;
		n = 0;
	}
	return rtn;
}

inline unsigned long shashfun2(const char* s) {
	if (!s) {
		return 0;
	}
	register unsigned long rtn = 0;
	register unsigned long n = 0;
	register int i = 0;
	for (register const char* p = s; *p; p++) {
		n = (n<<8)+(unsigned char)(*p);
		if (++i>=sizeof(n)) {
			rtn += n;
			n = 0;
			i = 0;
		}
	}
	if (n) {
		rtn += n;
	}
	return rtn%SHASH_PRIME;
}





//------------------------------------------------------------------
//
// String-Object Hash class.
//
//------------------------------------------------------------------

template < typename T >
struct SOBuck {
	string2		name;
	T			value;
	SOBuck*		next;

	void* operator new (size_t sz, void* addr) {
		return mmalloc_alloc(sz, addr);
	}

	void operator delete (void* addr) {
		mmalloc_free(addr);
	}
};




template <
	typename T,
	typename BuckType = SOBuck< T >,
	typename Alloc = mmalloc< BuckType >
>
class sohash {
public:
	typedef T					value_type;
	typedef BuckType 			Bucket;	// This is for outside of this class.
	typedef BuckType*			iter;
	typedef const BuckType* 	const_iter;

protected:
	// Member data
	Alloc		m_allocator;
	BuckType**	m_buckets;
	size_t		m_max;
	size_t		m_size;
	T			m_def;

protected:
	void free() {
		if (m_buckets) {
			clear();
			delete []m_buckets;
			m_buckets = NULL;
			m_max = 0;
			m_size = 0;
		}
	}

	#define getkey(name) (shashfun(name)%m_max)

	BuckType* getbucket(const char* name) const {
		register BuckType* bucket = m_buckets[getkey(name)];
		while (bucket) {
			if (bucket->name==name) {
				return bucket;
			}
			bucket = bucket->next;
		}
		return NULL;
	}

public:

	// Constructors & destructor
	sohash(size_t max = SHASH_DEFAULT_BUCK):
		m_buckets(new BuckType*[max]),
		m_max(max),
		m_size(0),
		m_def(T())
	{
		bzero(m_buckets, m_max*sizeof(*m_buckets));
	}

	sohash(const sohash& other):
		m_buckets(NULL),
		m_max(0),
		m_def(T())
	{
		*this = other;
	}

	sohash& operator =(const sohash& other) {
		if (this!=&other) {
			free();
			if ((m_max = other.m_max)>0 && other.m_buckets!=NULL) {
				if ((m_buckets = new BuckType*[m_max])!=NULL) {
					bzero(m_buckets, m_max*sizeof(*m_buckets));
					register const BuckType* bucket = NULL;
					for (register size_t i = 0; i<other.m_max; i++) {
						bucket = other.m_buckets[i];
						while (bucket) {
							set(bucket->name.c_str(), bucket->value);
							bucket = bucket->next;
						}
					}
				}
			}
		}
		return *this;
	}

	virtual ~sohash() {
		free();
	}

	const_iter find(const char* name) const {
		return getbucket(name);
	}

	const_iter find(const char* name, T& value) const {
		const_iter bucket = getbucket(name);
		if (bucket) {
			value = bucket->value;
		}
		return bucket;
	}

	// Operators
	const T& get(const char* name) const {
		const BuckType* bucket = getbucket(name);
		return bucket ? bucket->value : m_def;
	}

	T& get(const char* name) {
		BuckType* bucket = getbucket(name);
		return bucket ? bucket->value : m_def;
	}

	T& operator [](const char* name) {
		BuckType* bucket = getbucket(name);
		if (bucket) {
			return bucket->value;
		} else {
			if (NULL==(bucket = set(name, m_def))) {
				return m_def;
			} else {
				return bucket->value;
			}
		}
	}

	BuckType* set(const char* name, const T& value) {
		register unsigned long key = getkey(name);
		register BuckType* bucket = m_buckets[key];
		while (bucket) {
			if (bucket->name==name) {
				bucket->value = value;
				return bucket;
			}
			bucket = bucket->next;
		}

		// Not found, alloc new bucket
		if ((bucket = new(m_allocator.alloc()) BuckType)==NULL) {
			return NULL;
		}

		bucket->name = name;
		bucket->value = value;
		bucket->next = m_buckets[key];
		m_buckets[key] = bucket;
		m_size++;

		return bucket;
	}

	void del(const char* name) {
		register unsigned long key = getkey(name);
		register BuckType* prev = NULL;	// Pointer to previous bucket of current checking bucket
		register BuckType* bucket = m_buckets[key];
		while (bucket) {
			if (bucket->name==name) {
				if (bucket==m_buckets[key]) {
					m_buckets[key] = bucket->next;
				} else {
					prev->next = bucket->next;
				}
				delete bucket;
				m_size--;
				break;
			} else {
				prev = bucket;
				bucket = bucket->next;	// Move to next
			}
		}
	}

	void clear() {
		if (!m_buckets) {
			return;
		}
		register BuckType* next = NULL;
		for (register size_t i = 0; i<m_max; i++) {
			BuckType*& head = m_buckets[i];
			while (head) {
				next = head->next;
				delete head;
				head = next;
			}
		}
		m_size = 0;
	}

	const BuckType* buck(size_t index) const { return (index<m_max) ? m_buckets[index] : NULL; }

	size_t count_buck(size_t index) const {
		register size_t count = 0;
		const BuckType* bucket = buck(index);
		while (bucket) {
			count++;
			bucket = bucket->next;
		}
		return count;
	}

	size_t max_buck() const { return m_max; }

	/*
	*	Function:
	*		Count elements in this hash table.
	*/
	size_t size() const { return m_size; }

	void show_hash(FILE* fp) const {
		fprintf(fp, "--------------------------------------------\n");
		fprintf(fp, "Total %lu item(s)\n", size());
		if (m_buckets) {
			for (register size_t i = 0; i<m_max; i++) {
				const BuckType* bucket = buck(i);
				if (bucket) {
					fprintf(fp, "%010d : % 10d - ", i, count_buck(i));
					while (bucket) {
						fprintf(fp, "[%s] ", bucket->name.c_str());
						bucket = bucket->next;
					}
					fprintf(fp, "\n");
				}
			}
		}
		fprintf(fp, "--------------------------------------------\n");
	}
};


typedef sohash<string2> shash;

#endif

