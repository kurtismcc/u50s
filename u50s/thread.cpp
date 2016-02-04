#define VC_EXTRALEAN
#include <Windows.h>

#include "u50s/thread.h"

using namespace u50s;
using namespace u50s::containers;
using namespace u50s::synchronization;

typedef containers::intrusive_list<thread, &thread::m_link> thread_list;
static thread_list s_threads;
static synchronization::mutex s_mutex;
static thread_id_t s_last_thread_id;
thread_local thread_id_t thread::s_thread_id = thread_id_null;

static DWORD WINAPI thunk_thread_proc(void *param);

namespace u50s
{
class thread_impl
{
public:
	HANDLE m_handle;
	thread *m_actual;

	thread_impl(thread *t)
		:m_actual(t)
	{
		m_handle = CreateThread(nullptr, 0, &thunk_thread_proc, reinterpret_cast<void*>(this), CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr);
	}

	~thread_impl()
	{
		CloseHandle(m_handle);
	}

	uint32_t run()
	{
		m_actual->m_state = thread::STARTED;
		m_actual->m_return = m_actual->run();
		m_actual->m_state = thread::COMPLETED;
		return m_actual->m_return;
	}

	void start(uint32_t affinity_mask)
	{
		SetThreadAffinityMask(m_handle, affinity_mask);
		ResumeThread(m_handle);
	}

	void join()
	{
		WaitForSingleObject(m_handle, INFINITE);
	}
};
}

static DWORD WINAPI thunk_thread_proc(void *param)
{
	thread_impl *impl = reinterpret_cast<thread_impl*>(param);
	uint32_t ret_val = impl->run();
	return (DWORD)ret_val;
}

thread::thread()
	:m_link()
{
	scoped_acquire<mutex> lock(s_mutex);
	m_impl = new thread_impl(this);
	m_id = ++s_last_thread_id;
	m_state = CREATED;
	m_return = 0;
	s_threads.push_back(*this);
}

thread::~thread()
{
	scoped_acquire<mutex> lock(s_mutex);
	m_link.unlink();
	join();
	delete m_impl;
}



void thread::start(uint32_t affinity_mask)
{
	m_impl->start(affinity_mask);
}

void thread::join()
{
	m_impl->join();
}
