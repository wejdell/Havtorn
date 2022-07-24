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
		: NumberOfThreads(static_cast<U8>(std::thread::hardware_concurrency() - 1))
		, Terminate(false)
		, IsTerminated(false)
	{
	}

	CThreadManager::~CThreadManager()
	{
		if (!IsTerminated)
			Shutdown();
	}

	bool CThreadManager::Init(CRenderManager* renderManager)
	{
		RenderThread = std::thread(&CRenderManager::Render, renderManager);

		for (U8 i = 0; i < NumberOfThreads; ++i)
		{
			JobThreads.emplace_back(&CThreadManager::WaitAndPerformJobs, this);
		}
		return true;
	}

	void CThreadManager::WaitAndPerformJobs()
	{
		while (!Terminate)
		{
			// Code blocks used to unlock mutex when lock variables go out of scope - RAII
			{
				std::unique_lock<std::mutex> lock(QueueMutex);

				Condition.wait(lock, [this]()
				{
					return !JobQueue.empty() || Terminate;
				});

				if (!JobQueue.empty())
				{
					Job = JobQueue.front();
					JobQueue.pop();
					Job();
				}
			}

		}
	}

	void CThreadManager::PushJob(const std::function<void()>& job)
	{
		{
			std::unique_lock<std::mutex> lock(QueueMutex);
			JobQueue.push(job);
		}

		Condition.notify_one();
	}

	//#include <iostream>
	void CThreadManager::Shutdown()
	{
		{
			std::unique_lock<std::mutex> lock(ThreadPoolMutex);
			Terminate = true; // use this flag in condition.wait
		}

		Condition.notify_all(); // wake up all threads.

		// Join all threads.
		for (std::thread& thread : JobThreads)
		{
			thread.join();
		}

		JobThreads.clear();
		
		RunRenderThread = false;
		RenderThread.join();

		IsTerminated = true; // use this flag in destructor, if not set, call shutdown() 
	}
}
