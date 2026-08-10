// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "ThreadManager.h"
#include "Graphics/RenderManager.h"

namespace Havtorn
{
	std::mutex CThreadManager::RenderMutex;
	std::condition_variable CThreadManager::RenderCondition;
	ERenderThreadStatus CThreadManager::RenderThreadStatus = ERenderThreadStatus::ReadyToRender;
	bool CThreadManager::RunRenderThread = true;

	CThreadManager::CThreadManager()
		: NumberOfThreads(STATIC_U8(std::thread::hardware_concurrency() - 1))
		, Terminate(false)
	{
	}

	CThreadManager::~CThreadManager()
	{
		Terminate = true;
		Condition.notify_all();
		for (std::thread& thread : WorkerThreads)
			thread.join();
		
		WorkerThreads.clear();

		RunRenderThread = false;
		RenderCondition.notify_one();
		RenderThread.join();
	}

	bool CThreadManager::Init(CRenderManager* renderManager)
	{
		RenderThread = std::thread(&CRenderManager::Render, renderManager);

		for (U8 i = 0; i < NumberOfThreads; ++i)
		{
			WorkerThreads.emplace_back(&CThreadManager::WorkerLoop, this);
		}

		return true;
	}

	void CThreadManager::WorkerLoop()
	{
		while (!Terminate)
		{
			TaskSignature task;
			// Code blocks used to unlock mutex when lock variables go out of scope - RAII
			{
				std::unique_lock<std::mutex> lock(QueueMutex);

				Condition.wait(lock, [this]()
				{
					return Terminate || !TaskQueue.empty();
				});

				if (Terminate && TaskQueue.empty())
					return;
				
				task = std::move(TaskQueue.front());
				TaskQueue.pop();
			}
			task();
		}
	}

	void CThreadManager::Submit(TaskSignature task)
	{
		constexpr I32 maxQueueSize = 1000;

		{
			std::unique_lock<std::mutex> lock(QueueMutex);
			if (TaskQueue.size() >= maxQueueSize)
			{
				HV_LOG_WARN("CThreadManager::Submit: Task queue is full, can't push more tasks!");
				return;
			}
			TaskQueue.push(std::move(task));
		}

		Condition.notify_one();
	}

	void CThreadManager::ScheduleRepeatingTask(TaskSignature task, std::chrono::milliseconds interval, const std::optional<std::atomic<bool>*>& optionalStopRepeating)
	{
		std::atomic<bool>* stopRepeating = optionalStopRepeating.value_or(nullptr);
		Submit([=, this]() 
			{
				while (!Terminate && !(stopRepeating != nullptr && *stopRepeating))
				{
					Submit(task);
					std::this_thread::sleep_for(interval);
				}
			});
	}

	bool CThreadManager::StealTask(const U64& /*thiefID*/, TaskSignature& /*taskOut*/)
	{
		// NW: To make this work every thread should have its own std::dequeue<TaskSignature>, protected by a "lightweight spinlock or mutex"

		//for (U64 i = 0; i < WorkerThreads.size(); ++i)
		//{
		//	if (i == thiefID) 
		//		continue;

		//	std::unique_lock<std::mutex> lock(workerQueues[i].mutex);
		//	if (!workerqueues[i].queue.empty())
		//	{
		//		taskOut = std::move(workerQueues[i].queue.back());
		//		workerQueues[i].queue.pop_back();
		//		return true;
		//	}
		//}

		return false;
	}
}
