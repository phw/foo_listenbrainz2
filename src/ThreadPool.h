#pragma once

class SimpleThreadTask
{
public:
	virtual void run() = 0;
};

class SimpleThreadWorker : public pfc::thread
{
public:
	SimpleThreadWorker();
	virtual ~SimpleThreadWorker();

	void threadProc() override;

private:
	PFC_CLASS_NOT_COPYABLE_EX(SimpleThreadWorker)
};

class SimpleThreadPool
{
public:
	SimpleThreadPool();
	~SimpleThreadPool();

	static SimpleThreadPool& instance();

	SimpleThreadTask* acquire_task();
	bool enqueue(SimpleThreadTask* task);
	bool is_queue_empty();
	void exit();
	void remove_worker(SimpleThreadWorker* worker);
	void track(SimpleThreadTask* task);
	void untrack(SimpleThreadTask* task);
	void untrack_all();

	HANDLE empty_worker;
	HANDLE exiting;
	HANDLE have_task;

private:
	using TaskList = pfc::chain_list_v2_t<SimpleThreadTask*>;

	TaskList task_list;
	critical_section cs;
	pfc::counter num_workers = 0;

	PFC_CLASS_NOT_COPYABLE_EX(SimpleThreadPool)
};
