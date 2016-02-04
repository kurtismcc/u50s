#ifndef __U50S_MUTEX
#define __U50S_MUTEX

#include "u50s/synchronization/semaphore.h"
#include "u50s/system.h"
#include <stdexcept>
#include <sstream>

namespace u50s
{
namespace synchronization
{

class mutex
{
	semaphore m_sem;
	thread_id_t m_owner;

public:
	mutex() :m_sem(1), m_owner(thread_id_null) {}
	mutex(mutex &&m) :m_sem(std::move(m.m_sem)), m_owner(m.m_owner) {}

	// no assignment, no copying
	mutex(const mutex &) = delete;
	mutex &operator=(const mutex &) = delete;
	mutex &operator=(mutex &&m) = delete;

	bool try_acquire()
	{
		bool acquired = m_sem.try_acquire();
		if(acquired)
			m_owner = get_current_thread_id();
		return acquired;
	}

	void acquire()
	{
		thread_id_t requester = get_current_thread_id();
		m_sem.acquire();
		if(m_owner)  {
			std::ostringstream oss;
			oss << "u50s::synchronization::mutex::acquire (" << get_current_thread_id() << ", " << m_owner << ")";
			throw std::logic_error(oss.str());
		}
		m_owner = get_current_thread_id();
		return;
	}

	void release() throw(std::logic_error)
	{
		thread_id_t releaser = get_current_thread_id();
		if(releaser != m_owner)  {
			std::ostringstream oss;
			oss << "u50s::synchronization::mutex::release (" << releaser << ", " << m_owner << ")";
			throw std::logic_error(oss.str());
		}
		m_owner = thread_id_null;
		m_sem.post(1);
	}
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