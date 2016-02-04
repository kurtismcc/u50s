#ifndef _U50S_SYSTEM
#define _U50S_SYSTEM

#include <stdint.h>

namespace u50s
{
	typedef uint32_t thread_id_t;
	static const thread_id_t thread_id_null = 0;

	thread_id_t get_current_thread_id();
	void pause();

	inline uint64_t rdtsc() { return (uint64_t)__rdtsc(); }
}

#ifdef _WIN32

#include <emmintrin.h>

static inline void u50s::pause()
{
	_mm_pause();
}

#else
#endif

#endif
