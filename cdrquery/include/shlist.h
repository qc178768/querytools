// shlist.h
/*
* This file defines the String-Hash-List template class.
* History:
*	0001.01 2004-05-20 Mike Liu:	Create this file.
*	0001.02 2004-06-08 Mike Liu:	Add operator [].
*	0001.03 2004-06-08 Mike Liu:	Rearrange the find().
*	0001.04 2004-06-22 Mike Liu:	Purify codes.
*									Remove virtual from destructors.
*	0001.05 2004-08-17 Mike Liu:	Add none const version find() functions.
*									m_def should be initialize!
*	0001.06 2004-09-01 Mike Liu:	Add default bucket number.
*/

#ifndef __shlist_h__
#define __shlist_h__

#include "shash.h"
#include "objalloc.h"


#define SHLIST_DEFAULT_BUCKET		251



template < typename T >
struct shitem
{
	const char*	name;		// Key of the item
	T			value;		// Value of the item
	shitem*		next;		// Next item
	shitem*		prev;		// Previous item

	void* operator new (size_t sz, void* addr) {
		return mmalloc_alloc(sz, addr);
	}

	void operator delete (void* addr) {
		mmalloc_free(addr);
	}
};



template
<
	typename T,
	typename ListItem = shitem< T >,
	typename Hash = sohash< ListItem* >,
	typename Alloc = mmalloc< ListItem >
>
class shlist
{
	// Hash object
	Hash		m_hash;
	Alloc		m_allocator;

	// List data member
	ListItem*	m_head;
	ListItem*	m_tail;
	size_t		m_size;
	T			m_def;

public:
	typedef T						value_type;
	typedef ListItem*				iter;
	typedef const ListItem*			const_iter;
	typedef typename Hash::Bucket	HashItem;

protected:
	// Append allocated item to tail of the list.
	void list_insert(ListItem* item) {
		if (m_tail) {	// There's data in list
			item->prev = m_tail;
			item->next = NULL;
			m_tail->next = item;
		} else {		// Empty list
			item->prev = NULL;
			item->next = NULL;
			m_head = item;
		}
		m_tail = item;
		m_size++;
	}


	// Remove item from list, and delete the item
	void list_delete(ListItem* item) {
		ListItem* prev = item->prev;
		if (prev) {
			prev->next = item->next;
		} else {	// Head is item
			m_head = item->next;
		}

		ListItem* next = item->next;
		if (next) {
			next->prev = item->prev;
		} else {	// Tail is item
			m_tail = item->prev;
		}

		delete item;
		m_size--;
	}

	// Erase the whole list.
	void list_clear() {
		while (m_head) {
			list_delete(m_head);
		}
	}

	ListItem* alloc_item(const char* name, const T& value) {
		ListItem* pitem = new(m_allocator.alloc()) ListItem;
		if (pitem) {
			list_insert(pitem);
			HashItem* hitem = m_hash.set(name, pitem);
			pitem->name = hitem->name.c_str();
			pitem->value = value;
		}
		return pitem;
	}

public:
	shlist(int max_buck = SHLIST_DEFAULT_BUCKET):
		m_hash(max_buck),
		m_head(NULL),
		m_tail(NULL),
		m_size(0),
		m_def(T())
	{
		// Nothing to do here.
	}

	shlist(const shlist& other):
		m_hash(other.m_hash.max_buck()),
		m_head(NULL),
		m_tail(NULL),
		m_size(0),
		m_def(T())

	{
		*this = other;
	}

	shlist& operator = (const shlist& other)
	{
		if (this!=&other) {
			clear();
			for (const_iter it = other.begin(); it; it = it->next) {
				set(it->name, it->value);
			}
		}
		return *this;
	}

	~shlist()
	{
		clear();
	}

	const_iter find(const char* name) const
	{
		return m_hash.get(name);
	}

	iter find(const char* name)
	{
		return m_hash.get(name);
	}

	const_iter find(const char* name, T& value) const
	{
		const_iter it = find(name);
		if (it) {
			value = it->value;
		}
		return it;
	}

	iter find(const char* name, T& value)
	{
		iter it = find(name);
		if (it) {
			value = it->value;
		}
		return it;
	}

	iter set(const char* name, const T& value)
	{
		ListItem* pitem = m_hash.get(name);
		if (pitem) {
			pitem->value = value;
		} else {
			pitem = alloc_item(name, value);
		}
		return pitem;
	}

	T& get(const char* name)
	{
		ListItem* pitem = m_hash.get(name);
		if (pitem) {
			return pitem->value;
		} else {
			return m_def;
		}
	}

	const T& get(const char* name) const
	{
		const ListItem* pitem = m_hash.get(name);
		if (pitem) {
			return pitem->value;
		} else {
			return m_def;
		}
	}

	bool del(const char* name)
	{
		ListItem* pitem = m_hash.get(name);
		if (pitem) {
			m_hash.del(name);
			list_delete(pitem);
			return true;
		} else {
			return false;
		}
	}

	bool del(iter it)
	{
		if (it) {
			m_hash.del(it->name);
			list_delete(it);
			return true;
		} else {
			return false;
		}
	}

	T& operator [] (const char* name)
	{
		ListItem* pitem = m_hash.get(name);
		if (pitem) {
			return pitem->value;
		} else {
			if ((pitem = alloc_item(name, m_def))!=NULL) {
				return pitem->value;
			}
		}
		return m_def;
	}

	void clear()
	{
		while (m_head) {
			del(m_head);
		}
	}

	size_t size() const { return m_size; }

	bool empty() const { return size()==0; }

	iter begin() { return m_head; }

	const_iter begin() const { return m_head; }

	iter rbegin() { return m_tail; }

	const_iter rbegin() const { return m_tail; }

	void show_hash(FILE* fp) const {
		m_hash.show_hash(fp);
	}
};

#endif

