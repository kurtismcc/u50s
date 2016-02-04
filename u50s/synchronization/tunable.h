#ifndef _U50S_OPTIMISTIC
#define _U50S_OPTIMISTIC

#include <stdint.h>

namespace u50s 
{
namespace synchronization
{

constexpr uint32_t min_num_optimistic_spins = 32;
constexpr uint32_t max_num_optimistic_spins = 4096;

template <typename primative>
class tunable : public primative
{
	uint32_t m_fails;
	uint32_t m_success;
	uint32_t m_spins;

public:
	tunable() :m_fails(0), m_success(0), m_spins(min_num_optimistic_spins << 2) {}
	void acquire()
	{
		bool acquired = false;
		uint32_t count = m_spins;
		while(count)  {
			acquired = try_acquire();
			if(acquired)
				break;
			count--;
			pause();
		}
		if(!acquired)  {
			primative::acquire();
			m_fails++;
			uint32_t adjust = uint32_t(float(m_spins) * 0.32f);
			m_spins += adjust;
		} else {
			m_success++;
			uint32_t adjust = uint32_t(float(m_spins) * 0.03f);
			m_spins -= adjust;
		}
		if(m_spins < min_num_optimistic_spins)
			m_spins = min_num_optimistic_spins;
		if(m_spins > max_num_optimistic_spins)
			m_spins = max_num_optimistic_spins;
	}

	uint32_t fails() const { return m_fails; }
	uint32_t success() const { return m_success; }
	uint32_t spins() const { return m_spins; }
};

} // namespace synchronization
} // namespace u50s

#endif