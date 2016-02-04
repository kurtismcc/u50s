#ifndef __U50S_MUTEX
#define __U50S_MUTEX

#include "u50s/config.h"

namespace u50s
{
namespace synchronization
{

class mutex_impl; // mutex is pimpl-ed to prevent inclusion of lots of system headers

class mutex
{
	mutex_impl *m_impl;
public:
	mutex();
	~mutex();
		
	mutex(mutex &&m)
	{
		m_impl = m.m_impl;
		m.m_impl = nullptr;
	}

	// no assignment, no copying
	mutex(const mutex &) = delete;
	mutex &operator=(const mutex &) = delete;
	mutex &operator=(mutex &&m) = delete;

	bool try_acquire();
	void acquire();
	void release();
};

template <typename MUTEX>
class scoped_acquire
{
	MUTEX &m_mutex;
public:
	scoped_acquire(MUTEX &m) :m_mutex(m) { m_mutex.acquire(); }
	~scoped_acquire() { m_mutex.release(); }
};

} // namespace synchronization
} // namespace u50s

#endif