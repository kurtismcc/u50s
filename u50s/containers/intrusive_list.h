#ifndef __U50S_INTRUSIVE_LIST
#define __U50S_INTRUSIVE_LIST

#include <stdlib.h>

namespace u50s
{
namespace containers
{

class list_link
{
	list_link *m_prev;
	list_link *m_next;
public:
	list_link() :m_prev(this), m_next(this) {}
	~list_link() { unlink(); }

	void unlink()
	{
		m_next->m_prev = m_prev;
		m_prev->m_next = m_next;
		m_prev = this;
		m_next = this;
	}

	const list_link *prev() const { return m_prev; }
	list_link *prev() { return m_prev; }
	const list_link *next() const { return m_next; }
	list_link *next() { return m_next; }

	void insert(list_link *next)
	{
		unlink();

		m_prev = next->m_prev;
		m_next = next;
		next->m_prev->m_next = this;
		next->m_prev = this;
	}
};

template <typename T, list_link T::* link_var>
class intrusive_list
{
	list_link m_tail;

	static const list_link *get_link(const T* ptr)
	{
		return &(ptr->*link_var);
	}

	static list_link *get_link(T *ptr)
	{
		return const_cast<list_link*>(get_link(const_cast<const T*>(ptr)));
	}

	static const T *get_val(const list_link *link)
	{
		static const T* dummy = reinterpret_cast<const T*>(0);
		static const ptrdiff_t offset = reinterpret_cast<const char*>(&(dummy->*link_var)) - reinterpret_cast<const char*>(dummy);
		const char *raw = reinterpret_cast<const char *>(link) - offset;
		return reinterpret_cast<const T*>(raw);
	}

	static T *get_val(list_link *link)
	{
		return const_cast<T*>(get_val(link));
	}

public:

	class const_iterator
	{
		const list_link *m_link;
	public:
		const_iterator() :m_link(nullptr) {}
		const_iterator(const list_link *link) :m_link(link) {}
		const_iterator(const const_iterator &it) :m_link(it.m_link) {}

		const T* operator->() const
		{
			return get_val(m_link);
		}

		const T& operator*() const
		{
			return *get_val(m_link);
		}

		const_iterator operator++()
		{
			m_link = m_link->next();
			return *this;
		}

		const_iterator operator++(int)
		{
			const_iterator it(*this);
			operator++();
			return it;
		}

		const_iterator operator--()
		{
			m_link = m_link->prev();
			return *this;
		}

		const_iterator operator--(int)
		{
			const_iterator it(*this);
			operator--();
			return it;
		}

		const_iterator operator+=(ptrdiff_t n)
		{
			if(n > 0) {
				while(n) {
					++(*this);
					--n;
				}
			} else {
				while(n) {
					--(*this);
					++n;
				}
			}
			return *this;
		}

		const_iterator operator-=(ptrdiff_t n)
		{
			return this->operator+=(-n);
		}

		const_iterator operator+(ptrdiff_t n) const
		{
			const_iterator it(*this);
			it += n;
			return it;
		}

		const_iterator operator-(ptrdiff_t n) const
		{
			const_iterator it(*this);
			it -= n;
			return it;
		}

		bool operator==(const const_iterator &it) const
		{
			return m_link == it.m_link;
		}

		bool operator!=(const const_iterator &it) const
		{
			return m_link != it.m_link;
		}
	};

	class iterator : public const_iterator
	{
	public:
		iterator() {}
		iterator(list_link *link) :const_iterator(link) {}
		iterator(const iterator &it) :const_iterator(it) {}

		T* operator->()
		{
			return const_cast<T*>(this->const_iterator::operator->());
		}

		T& operator*()
		{
			return const_cast<T&>(this->const_iterator::operator*());
		}

		iterator operator++()
		{
			return static_cast<iterator&>(this->const_iterator::operator++());
		}

		iterator operator++(int)
		{
			return static_cast<iterator&>(this->const_iterator::operator++(0));
		}

		iterator operator--()
		{
			return static_cast<iterator&>(this->const_iterator::operator--());
		}

		iterator operator--(int)
		{
			return static_cast<iterator&>(this->const_iterator::operator--(0));
		}

		iterator operator+=(ptrdiff_t n)
		{
			return static_cast<iterator&>(this->const_iterator::operator+=(n));
		}

		iterator operator-=(ptrdiff_t n)
		{
			return static_cast<iterator&>(this->const_iterator::operator-=(n));
		}

		iterator operator+(ptrdiff_t n) const
		{
			return static_cast<iterator&>(this->const_iterator::operator+(n));
		}

		iterator operator-(ptrdiff_t n) const
		{
			return static_cast<iterator&>(this->const_iterator::operator-(n));
		}

		bool operator==(const iterator &it) const
		{
			return this->const_iterator::operator==(it);
		}

		bool operator!=(const iterator &it) const
		{
			return this->const_iterator::operator!=(it);
		}
	};

	iterator begin() { return iterator(m_tail.next()); }
	const_iterator begin() const { return const_iterator(m_tail.next()); }

	iterator end() { return iterator(&m_tail); }
	const_iterator end() const { return const_iterator(&m_tail); }

	void unlink_all()
	{
		const iterator c_end = end();
		for(iterator it = begin(); c_end != it;)  {
			iterator unlinkMe(it++);
			get_link(&*unlinkMe)->unlink();
		}
	}

	intrusive_list() {}
	~intrusive_list()
	{
		unlink_all();
	}

	const T& front() const { return *begin(); }
	T& front() { return const_cast<T&>(front()); }

	const T& back() const
	{
		const_iterator it = end();
		--it;
		return *it;
	}
	T& back() { return const_cast<T&>(back()); }

	void push_front(T &val)
	{
		list_link *link = get_link(&val);
		link->insert(m_tail.next());
	}

	void push_back(T &val)
	{
		list_link *link = get_link(&val);
		link->insert(&m_tail);
	}

	bool empty() const
	{
		return begin() == end();
	}

	void erase(iterator it)
	{
		get_link(&*it)->unlink();
	}

	void erase(const_iterator it)
	{
		erase(static_cast<iterator&>(it));
	}
};

} // namespace containers
} // namespace u50s

#endif