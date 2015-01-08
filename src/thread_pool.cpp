#include <assert.h>
#include "thread_pool.h"
#include "common.h"


/*
 * thread_pool
 */

thread_pool::thread_pool(int nthreads) : 
	nthreads(nthreads ? nthreads : std::thread::hardware_concurrency()),
	exit_flag(false)
{
}

thread_pool::~thread_pool()
{
	stop();
}

void thread_pool::run(bool forever)
{
	run_async(forever);
	wait();
	threads.clear();
}

void thread_pool::run_async(bool forever)
{
	assert(threads.empty());
	threads.reserve(nthreads);

	exit_after_completion = !forever;

	for (int i = 0; i < nthreads; ++ i)
	{
		threads.push_back(
			std::thread(&thread_pool::loop, std::ref(*this))
		);
	}
}

void thread_pool::stop()
{
	notify_stop();
	wait();
	threads.clear();
}

void thread_pool::wait()
{
	for (size_t i = 0; i < threads.size(); ++ i)
	{
		if (threads[i].joinable())
			threads[i].join();
	}
}

bool thread_pool::pop_task(thread_pool::task& t)
{
	std::unique_lock<std::mutex> lock(queue_mutex);

	if (exit_after_completion)
	{
		if (exit_flag)
			return false;

		if (task_queue.empty())
		{
			notify_stop();
			return false;
		}
	}
	else
	{
		while (true)
		{
			if (exit_flag)
				return false;
	
			if (!task_queue.empty())
				break;

			queue_notifier.wait(lock);
		}
	}

	t = std::move(task_queue.front());
	task_queue.pop();

	return true;
}

void thread_pool::push_task(thread_pool::task t)
{
	std::unique_lock<std::mutex> lock(queue_mutex);

	task_queue.push(std::move(t));
	queue_notifier.notify_one();
}

void thread_pool::loop()
{
	while (true)
	{
		try
		{
			task t;
			if (!pop_task(t))
				break;
			t();
		}
		catch (std::exception const& e)
		{
			std::cout << get_thread_id() + " failed: " + e.what() + "\n";
		}
	}
}

void thread_pool::post(thread_pool::task t)
{
	push_task(t);
}

void thread_pool::notify_stop()
{
	exit_flag = true;
	queue_notifier.notify_all();
}
