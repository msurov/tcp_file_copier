#ifndef __thread_pool_h__
#define __thread_pool_h__

#include <queue>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "trace.h"


/*
 * thread_pool
 */

class thread_pool
{
private:
	typedef std::function<void()>	task;

	int const 					nthreads;
	std::vector<std::thread> 	threads;
	std::queue<task>			task_queue;
	bool 						exit_after_completion;
	bool						exit_flag;
	std::mutex					queue_mutex;
	std::condition_variable		queue_notifier;

	// blocking routine
	bool pop_task(task& t);
	void push_task(task t);

	// main task loop
	void loop();

	void notify_stop();

public:
	thread_pool(int nthreads = 0);
	~thread_pool();

	// pass a task object to thread pool
	// after task completes the object will be 
	// deleted
	void post(task t);

	// blocking calls
	void run(bool forever = false);
	void stop();
	void wait();

	// non-blocking
	void run_async(bool forever = false);
};

#endif // __thread_pool_h__
