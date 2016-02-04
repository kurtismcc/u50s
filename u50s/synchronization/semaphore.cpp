#define VC_EXTRALEAN
#include <Windows.h>

#include "u50s/synchronization/semaphore.h"

namespace u50s 
{
namespace synchronization
{

class semaphore_impl
{
	HANDLE m_sem;
public:
	semaphore_impl()
	{
		m_sem = CreateSemaphore(nullptr, 0, 1, nullptr);
	}

	~semaphore_impl()
	{
		CloseHandle(m_sem);
	}

	void acquire()
	{
		WaitForSingleObject(m_sem, INFINITE);
	}

	void post(uint32_t n)
	{
		LONG dummy;
		ReleaseSemaphore(m_sem, n, &dummy);
	}
};

}
}

using namespace u50s::synchronization;

semaphore::semaphore(uint32_t n)
	:m_val(n)
{
	m_impl = new semaphore_impl;
}

semaphore::~semaphore()
{
	delete m_impl;
	m_impl = nullptr;
}

void semaphore::acquire()
{
	int32_t val = m_val.fetch_sub(1);
	if(val > 0)
		return;

	m_impl->acquire();
}

void semaphore::post(uint32_t n)
{
	int32_t val = m_val.fetch_add(n);
	if(val >= 0)
		return;

	uint32_t num_asleep = -val;
	m_impl->post(num_asleep > n ? n : num_asleep);
}
