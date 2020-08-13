#include "stdafx.h"
#include "ThreadPool.h"

namespace
{
	class InitQuit : public initquit
	{
		void on_quit() override
		{
			SimpleThreadPool::instance().exit();
		}
	};

	FB2K_SERVICE_FACTORY(InitQuit);
}

SimpleThreadWorker::SimpleThreadWorker() {}

SimpleThreadWorker::~SimpleThreadWorker()
{
	waitTillDone();
}

void SimpleThreadWorker::threadProc()
{
	uint64_t last_tick = GetTickCount64();

	while (WaitForSingleObject(SimpleThreadPool::instance().exiting, 0) == WAIT_TIMEOUT)
	{
		if (WaitForSingleObject(SimpleThreadPool::instance().have_task, 1000) == WAIT_OBJECT_0)
		{
			auto task = SimpleThreadPool::instance().acquire_task();

			if (task)
			{
				task->run();
				SimpleThreadPool::instance().untrack(task);
				last_tick = GetTickCount64();
				continue;
			}
		}

		if (GetTickCount64() - last_tick >= 10000)
		{
			if (SimpleThreadPool::instance().is_queue_empty()) break;
		}
	}

	SimpleThreadPool::instance().remove_worker(this);
}

SimpleThreadPool::SimpleThreadPool()
{
	empty_worker = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	exiting = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	have_task = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	pfc::dynamic_assert(empty_worker != INVALID_HANDLE_VALUE);
	pfc::dynamic_assert(exiting != INVALID_HANDLE_VALUE);
	pfc::dynamic_assert(have_task != INVALID_HANDLE_VALUE);
}

SimpleThreadPool::~SimpleThreadPool()
{
	CloseHandle(empty_worker);
	CloseHandle(exiting);
	CloseHandle(have_task);
}

SimpleThreadPool& SimpleThreadPool::instance()
{
	static SimpleThreadPool instance;
	return instance;
}

SimpleThreadTask* SimpleThreadPool::acquire_task()
{
	insync(cs);

	auto iter = task_list.first();

	if (iter.is_valid())
	{
		task_list.remove(iter);
	}

	if (is_queue_empty())
		ResetEvent(have_task);

	return iter.is_valid() ? *iter : nullptr;
}

bool SimpleThreadPool::enqueue(SimpleThreadTask* task)
{
	if (WaitForSingleObject(exiting, 0) == WAIT_OBJECT_0)
		return false;

	insync(cs);
	const int max_count = pfc::getOptimalWorkerThreadCount();
	track(task);

	if (num_workers < max_count)
	{
		auto worker = new SimpleThreadWorker;
		worker->start();
		++num_workers;
		ResetEvent(empty_worker);
	}

	return true;
}

bool SimpleThreadPool::is_queue_empty()
{
	insync(cs);
	return task_list.get_count() == 0;
}

void SimpleThreadPool::exit()
{
	core_api::ensure_main_thread();

	SetEvent(exiting);

	// Because the tasks may use blocking SendMessage() function, it should be avoid using
	// infinite wait here, or both main thread and worker thread will block, and it's also
	// important to dispatch windows messages here.
	while (WaitForSingleObject(empty_worker, 0) == WAIT_TIMEOUT)
	{
		MSG msg;

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	untrack_all();
}

void SimpleThreadPool::remove_worker(SimpleThreadWorker* worker)
{
	insync(cs);
	--num_workers;

	if (num_workers == 0)
		SetEvent(empty_worker);

	fb2k::inMainThread([=]()
		{
			delete worker;
		});
}

void SimpleThreadPool::track(SimpleThreadTask* task)
{
	insync(cs);
	const bool empty = is_queue_empty();
	task_list.add_item(task);

	if (empty)
		SetEvent(have_task);
}

void SimpleThreadPool::untrack(SimpleThreadTask* task)
{
	insync(cs);
	task_list.remove_item(task);
	delete task;

	if (is_queue_empty())
		ResetEvent(have_task);
}

void SimpleThreadPool::untrack_all()
{
	insync(cs);
	for (TaskList::iterator iter = task_list.first(); iter.is_valid(); ++iter)
	{
		task_list.remove(iter);
		delete *iter;
	}

	ResetEvent(have_task);
}
