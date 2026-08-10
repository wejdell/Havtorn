// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <future>
#include <chrono>

namespace Havtorn
{
	enum class ERenderThreadStatus
	{
		ReadyToRender,
		PostRender,
		Count
	};

	class CRenderManager;

	using TaskSignature = std::function<void()>;

	class CThreadManager
	{
	public:
		CThreadManager();
		~CThreadManager();
		CThreadManager(const CThreadManager&) = delete;
		CThreadManager(const CThreadManager&&) = delete;
		CThreadManager operator=(const CThreadManager&) = delete;
		CThreadManager operator=(const CThreadManager&&) = delete;

		bool Init(CRenderManager* renderManager);
		void WorkerLoop();
		
		ENGINE_API void Submit(TaskSignature task);

		template<typename Function, typename... Args>
		auto SubmitTask(Function&& function, Args&&... args) -> std::future<decltype(function(args...))>
		{
			using ReturnType = decltype(function(args...));
			auto jobPtr = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<Function>(function), std::forward<Args>(args)...));

			std::future<ReturnType> result = jobPtr->get_future();
			Submit([jobPtr]() { (*jobPtr)(); });

			return result;
		}

		ENGINE_API void ScheduleRepeatingTask(TaskSignature task, std::chrono::milliseconds interval, const std::optional<std::atomic<bool>*>& optionalStopRepeating = {});

		ENGINE_API bool StealTask(const U64& thiefID, TaskSignature& taskOut);

		static std::mutex RenderMutex;
		static std::condition_variable RenderCondition;
		static ERenderThreadStatus RenderThreadStatus;

	private:
		friend class CRenderManager;
		static bool RunRenderThread;
		
		std::vector<std::thread> WorkerThreads;
		std::queue<TaskSignature> TaskQueue;

		std::thread RenderThread;

		std::mutex QueueMutex;
		std::condition_variable Condition;
		std::atomic<bool> Terminate = false;

		U8 NumberOfThreads = 0;
	};
}
