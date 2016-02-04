#ifndef _U50S_SPIN_MUTEX
#define _U50S_SPIN_MUTEX

#include "u50s/system.h"
#include <atomic>
#include <sstream>

namespace u50s
{
namespace synchronization
{

class spin_mutex
{
	std::atomic<thread_id_t> m_owner;

public:
	spin_mutex() :m_owner(0) {}

	~spin_mutex() throw(std::logic_error)
	{
		if(0 != m_owner)  {
			std::ostringstream oss;
			oss << "u50s::synchronization::spin_mutex (" << m_owner << ")";
			throw std::logic_error(oss.str());
		}
	}

	bool try_acquire()
	{
		thread_id_t requester = get_current_thread_id();
		thread_id_t actual = thread_id_null;
		return m_owner.compare_exchange_strong(actual, requester);
	}

	void acquire()
	{
		thread_id_t requester = get_current_thread_id();
		thread_id_t actual = thread_id_null;
		while(!m_owner.compare_exchange_weak(actual, requester))
			pause();
	}

	void release()
	{
		m_owner.store(thread_id_null);
	}
};

} // namespace synchronization
} // namespace u50s

#endif
