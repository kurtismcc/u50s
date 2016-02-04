#ifndef _U50S_SEMAPHORE
#define _U50S_SEMAPHORE

#include <stdint.h>
#include <atomic>
#include "u50s/system.h"

namespace u50s 
{
namespace synchronization
{

class semaphore_impl; // kernel part is pimpl-ed to prevent inclusion of lots of system headers

class semaphore
{
	semaphore_impl *m_impl;
	std::atomic<int32_t> m_val;

public:
	semaphore(uint32_t n);
	~semaphore();

	semaphore(semaphore &&m)
	{
		m_impl = m.m_impl;
		m_val.exchange(m.m_val);
		m.m_impl = nullptr;
	}

	// no assignment, no copying
	semaphore(const semaphore &) = delete;
	semaphore &operator=(const semaphore &) = delete;
	semaphore &operator=(semaphore &&) = delete;

	bool try_acquire()
	{
		int32_t old_val;
		do {
			pause();
			old_val = m_val;
			if(old_val <= 0)
				return false;
		} while(!m_val.compare_exchange_weak(old_val, old_val - 1));
		return true;
	}

	void acquire();
	void post(uint32_t n);

};

} // namespace synchronization
} // namespace u50s

#endif