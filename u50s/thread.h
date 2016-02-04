#ifndef _U50S_THREAD
#define _U50S_THREAD

#include <stdint.h>
#include "u50s/containers/intrusive_list.h"
#include "u50s/synchronization/mutex.h"

namespace u50s
{

class thread_impl;

class thread
{
public:
	enum thread_state
	{
		CREATED,
		STARTED,
		STOPPING,
		COMPLETED
	};
	containers::list_link m_link;

private:
	friend class thread_impl;

	thread_impl *m_impl;
	thread_id_t m_id;
	thread_state m_state;
	uint32_t m_return;

	static thread_local thread_id_t s_thread_id;

public:
	thread();
	~thread();

	// no assignment, no copying, no moving
	thread(const thread &) = delete;
	thread(thread &&m) = delete;
	thread &operator=(const thread &) = delete;
	thread &operator=(thread &&m) = delete;

	virtual uint32_t run() = 0;

	void start(uint32_t affinity_mask = ~0u);
	void join();

	thread_state state() { return m_state; }
	thread_id_t id() { return m_id; }
	static thread_id_t current_thread_id() { return s_thread_id; }
};

} // namespace u50s

#endif