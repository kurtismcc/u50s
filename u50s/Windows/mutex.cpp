#define VC_EXTRALEAN
#include <Windows.h>

#include "u50s/synchronization/mutex.h"

namespace u50s
{
namespace synchronization
{

class mutex_impl
{
	CRITICAL_SECTION m_critsect;
public:
	mutex_impl() 
	{
		InitializeCriticalSection(&m_critsect);
	}

	~mutex_impl()
	{
		DeleteCriticalSection(&m_critsect);
	}

	bool try_acquire()
	{
		BOOL acquired = TryEnterCriticalSection(&m_critsect);
		return FALSE != acquired;
	}

	void acquire()
	{
		EnterCriticalSection(&m_critsect);
	}

	void release()
	{
		LeaveCriticalSection(&m_critsect);
	}
};

}
}

using namespace u50s;
using namespace u50s::synchronization;

mutex::mutex()
{
	m_impl = new mutex_impl;
}

mutex::~mutex()
{
	delete m_impl;
	m_impl = nullptr;
}

bool mutex::try_acquire()
{
	return m_impl->try_acquire();
}

void mutex::acquire()
{
	return m_impl->acquire();
}

void mutex::release()
{
	return m_impl->release();
}