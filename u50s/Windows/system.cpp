#define VC_EXTRALEAN
#include <Windows.h>

#include "u50s/system.h"
#include "u50s/thread.h"

using namespace u50s;

thread_id_t u50s::get_current_thread_id()
{
	return (uint32_t)GetCurrentThreadId();
}