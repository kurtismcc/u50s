#define VC_EXTRALEAN
#include <Windows.h>

#include "u50s/containers/intrusive_list.h"
#include "u50s/synchronization/mutex.h"
#include "u50s/synchronization/recursive_mutex.h"
#include "u50s/synchronization/spin_mutex.h"
#include "u50s/synchronization/tunable.h"
#include "u50s/thread.h"

#include <iostream>

using namespace u50s;
using namespace u50s::containers;
using namespace u50s::synchronization;

struct node
{
	int n;
	list_link link;
};

void test_intrusive_list()
{
	intrusive_list<node, &node::link>  my_list;

	node n1, n2, n3, n4, n5, n6;
	n1.n = 0;
	n2.n = 1;
	n3.n = 2;
	n4.n = 3;
	n5.n = 4;
	n6.n = 5;

	my_list.push_back(n1);
	my_list.push_back(n2);
	my_list.push_back(n3);
	my_list.push_back(n4);
	my_list.push_back(n5);
	my_list.push_back(n6);

	for(const auto &it : my_list)
	{
		std::cout << it.n << " ";
	}

	std::cout << std::endl;

	my_list.unlink_all();

	my_list.push_back(n6);
	my_list.push_back(n5);
	my_list.push_back(n4);
	my_list.push_back(n3);
	my_list.push_back(n2);
	my_list.push_back(n1);

	for(const auto &it : my_list)
	{
		std::cout << it.n << " ";
	}

	std::cout << std::endl;

	my_list.unlink_all();

	my_list.push_front(n1);
	my_list.push_front(n2);
	my_list.push_front(n3);
	my_list.push_front(n4);
	my_list.push_front(n5);
	my_list.push_front(n6);

	for(const auto &it : my_list)
	{
		std::cout << it.n << " ";
	}

	std::cout << std::endl;
	
	my_list.unlink_all();

	my_list.push_front(n6);
	my_list.push_front(n5);
	my_list.push_front(n4);
	my_list.push_front(n3);
	my_list.push_front(n2);
	my_list.push_front(n1);

	for(const auto &it : my_list)
	{
		std::cout << it.n << " ";
	}

	std::cout << std::endl;

	my_list.unlink_all();

	my_list.push_front(n6);
	my_list.push_front(n5);
	my_list.push_front(n4);
	my_list.push_front(n3);
	my_list.push_front(n2);
	my_list.push_front(n1);

	my_list.erase(my_list.begin() + 3);

	for(const auto &it : my_list)
	{
		std::cout << it.n << " ";
	}

	std::cout << std::endl;

	my_list.erase(my_list.end() - 2);

	for(const auto &it : my_list)
	{
		std::cout << it.n << " ";
	}

	std::cout << std::endl;

	my_list.unlink_all();
}

void test_mutex()
{
	mutex m;

	m.acquire();
	m.release();
	
	m.acquire();
	if(m.try_acquire())
		throw std::logic_error("test failed"); // non-recursive shouldn't be able to grab recursively
	m.release();

	{
		scoped_acquire<mutex> lock(m);
	}
}

void test_recursive_mutex()
{
	recursive_mutex<mutex> rm;
	
	rm.acquire();
	rm.release();

	rm.acquire();
	rm.acquire();
	rm.acquire();
	rm.release();
	rm.release();
	rm.release();

	rm.acquire();
	rm.acquire();
	if(!rm.try_acquire())
		throw std::logic_error("test failed"); // recursive mutex should be able to grab recursively
	rm.release();
	rm.release();
	rm.release();

	{
		scoped_acquire<recursive_mutex<mutex>> lock(rm);
	}
}

void test_spin_mutex()
{
	spin_mutex sm;
	
	sm.acquire();
	sm.release();

	sm.acquire();
	if(sm.try_acquire())
		throw std::logic_error("test failed"); // non-recursive shouldn't be able to grab recursively
	sm.release();

	{
		scoped_acquire<spin_mutex> lock(sm);
	}

	recursive_mutex<spin_mutex> rsm;

	rsm.acquire();
	rsm.release();

	rsm.acquire();
	rsm.acquire();
	rsm.acquire();
	rsm.release();
	rsm.release();
	rsm.release();

	rsm.acquire();
	rsm.acquire();
	if(!rsm.try_acquire())
		throw std::logic_error("test failed"); // recursive mutex should be able to grab recursively
	rsm.release();
	rsm.release();
	rsm.release();

	{
		scoped_acquire<recursive_mutex<spin_mutex>> lock(rsm);
	}
}

mutex mut;
tunable<mutex> opt_mut;
std::atomic<uint32_t> stop;

class test_mut_thread : public thread
{
	uint32_t m_count;
public:
	test_mut_thread() :m_count(0) {}

	virtual uint32_t run()
	{
		while(0 == stop.load())  {
			scoped_acquire<mutex> lock(mut);
			m_count++;
		}
		return 0;	
	}

	uint32_t count() { return m_count; }
};

class test_mut2_thread : public thread
{
	uint32_t m_count;
public:
	test_mut2_thread() :m_count(0)	{}

	virtual uint32_t run()
	{
		while(0 == stop.load())  {
			scoped_acquire<tunable<mutex>> lock(opt_mut);
			m_count++;
		}
		return 0;	
	}

	uint32_t count() { return m_count; }
};

void test_contention()
{
	test_mut_thread t1;
	test_mut_thread t2;
	test_mut_thread t3;
	test_mut_thread t4;
	test_mut_thread t5;
//	test_mut_thread t6;
//	test_mut_thread t7;
//	test_mut_thread t8;

	stop.store(0);

	t1.start();
	t2.start();
	t3.start();
	t4.start();
	t5.start();
//	t6.start();
//	t7.start();
//	t8.start();

	Sleep(5000);
	stop.store(1);

	t1.join();
	std::cout << "thread " << t1.id() << ": " << t1.count() << std::endl;
	t2.join();
	std::cout << "thread " << t2.id() << ": " << t2.count() << std::endl;
	t3.join();
	std::cout << "thread " << t3.id() << ": " << t3.count() << std::endl;
	t4.join();
	std::cout << "thread " << t4.id() << ": " << t4.count() << std::endl;
	t5.join();
	std::cout << "thread " << t5.id() << ": " << t5.count() << std::endl;
//	t6.join();
//	std::cout << "thread " << t6.id() << ": " << t6.count() << std::endl;
//	t7.join();
//	std::cout << "thread " << t7.id() << ": " << t7.count() << std::endl;
//	t8.join();
//	std::cout << "thread " << t8.id() << ": " << t8.count() << std::endl;
}

void test_contention2()
{
	test_mut2_thread t1;
	test_mut2_thread t2;
	test_mut2_thread t3;
	test_mut2_thread t4;
	test_mut2_thread t5;
//	test_mut2_thread t6;
//	test_mut2_thread t7;
//	test_mut2_thread t8;

	stop.store(0);

	t1.start();
	t2.start();
	t3.start();
	t4.start();
	t5.start();
//	t6.start();
//	t7.start();
//	t8.start();

	Sleep(5000);
	stop.store(1);

	t1.join();
	std::cout << "thread " << t1.id() << ": " << t1.count() << std::endl;
	t2.join();
	std::cout << "thread " << t2.id() << ": " << t2.count() << std::endl;
	t3.join();
	std::cout << "thread " << t3.id() << ": " << t3.count() << std::endl;
	t4.join();
	std::cout << "thread " << t4.id() << ": " << t4.count() << std::endl;
	t5.join();
	std::cout << "thread " << t5.id() << ": " << t5.count() << std::endl;
//	t6.join();
//	std::cout << "thread " << t6.id() << ": " << t6.count() << std::endl;
//	t7.join();
//	std::cout << "thread " << t7.id() << ": " << t7.count() << std::endl;
//	t8.join();
//	std::cout << "thread " << t8.id() << ": " << t8.count() << std::endl;

	std::cout << "fails: " << opt_mut.fails() << std::endl;
	std::cout << "success: " << opt_mut.success() << std::endl;

	double percent = double(opt_mut.fails()) / double(opt_mut.fails() + opt_mut.success());
	std::cout << "percent: " << percent << std::endl;
	std::cout << "spins: " << opt_mut.spins() << std::endl;
}

constexpr uint32_t CACHE_LINE_SIZE = 4; // the documentation for x64 says 64, but Jump tested at 128... should probably reverify

template <typename T>
class cache_line_padding
{
	char buf[CACHE_LINE_SIZE - sizeof(T)];
};

struct interthread_signal
{
	std::atomic<uint32_t> m_val;
	cache_line_padding<std::atomic<uint32_t>> m_padding;
	interthread_signal() :m_val(0) {}
};

struct time_interthread_communication_thread : public thread
{
	uint32_t m_last_val;
	uint64_t m_total_lag;
	interthread_signal &m_signal_in;
	interthread_signal &m_signal_out;
public:
	time_interthread_communication_thread(interthread_signal &sin, interthread_signal &sout) :m_last_val(0), m_total_lag(0), m_signal_in(sin), m_signal_out(sout) {}

	virtual uint32_t run()
	{
		uint64_t start = rdtsc();
		while(0 == stop.load())  {
			uint32_t val;
			uint32_t old_val = m_last_val;
			std::atomic<uint32_t> *val_loc = &m_signal_in.m_val;
			do {
				val = val_loc->load();
				pause();
				pause();
			} while(val == old_val);
			
			m_last_val = val;
			m_signal_out.m_val.store(m_signal_out.m_val + 1);
		}
		m_total_lag = rdtsc() - start;
		return 0;
	}

	uint32_t last_val() const { return m_last_val; }
	uint64_t total_lag() const { return m_total_lag; }
};

void test_time_interthread_communication(uint32_t affinity_mask_1, uint32_t affinity_mask_2)
{
	interthread_signal s1, s2;
	time_interthread_communication_thread t1(s1, s2);
	time_interthread_communication_thread t2(s2, s1);

	stop.store(0);

	t1.start(affinity_mask_1);
	t2.start(affinity_mask_2);

	Sleep(500);
	
	s1.m_val.store(1);

	Sleep(3000);
	stop.store(1);

	t1.join();
	t2.join();

	uint64_t total_lag = t1.total_lag() + t2.total_lag();
	uint32_t total_times = t1.last_val() + t2.last_val();

	std::cout << total_lag << " ticks for " << total_times << " ping pongs: " << double(total_lag) / double(total_times) << " cycles/pong" << std::endl;
}

int main()
{
	test_intrusive_list();
	test_mutex();
	test_recursive_mutex();
	test_spin_mutex();

	test_contention();
	test_contention2();

	test_time_interthread_communication(1 << 0, 1 << 1);
	test_time_interthread_communication(1 << 2, 1 << 3);
	test_time_interthread_communication(1 << 4, 1 << 5);
	test_time_interthread_communication(1 << 6, 1 << 7);

	test_time_interthread_communication(1 << 0, 1 << 2);
	test_time_interthread_communication(1 << 1, 1 << 3);
	test_time_interthread_communication(1 << 4, 1 << 6);
	test_time_interthread_communication(1 << 5, 1 << 7);

	test_time_interthread_communication(1 << 0, 1 << 4);
	test_time_interthread_communication(1 << 1, 1 << 5);
	test_time_interthread_communication(1 << 2, 1 << 6);
	test_time_interthread_communication(1 << 3, 1 << 7);

	int x;
	std::cin >> x;

	return 0;
}