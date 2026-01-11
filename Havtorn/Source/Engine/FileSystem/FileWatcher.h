// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include <functional>
#include <windows.h>
#include <thread>
#include <mutex>
#include <filesystem>
#include <queue>

namespace fs = std::filesystem;

namespace Havtorn
{
	class CThreadManager;

	struct SFileChangeCallback
	{
		SFileChangeCallback() = delete;
		explicit SFileChangeCallback(const std::function<void(const std::string&)>& function, const U64& handle)
			: Function(function)
			, Handle(handle)
		{}

		std::function<void(const std::string&)> Function;
		U64 Handle = 0;
	};

	typedef std::function<void(const std::string&)> CFileChangeCallback;
	
	class CFileWatcher
	{
	public:
		CFileWatcher() = default;
		~CFileWatcher();

		bool Init(CThreadManager* threadManager);

		ENGINE_API bool WatchFileChange(const std::string& filePath, SFileChangeCallback callback);
		ENGINE_API void StopWatchFileChange(const std::string& filePath, const U64 callbackHandle);
		
		void FlushChanges(); 

	private:
		void UpdateChanges(); 

		std::map<fs::path, U64> WatchedFiles;
		
		std::map<fs::path, std::vector<SFileChangeCallback>> StoredCallbacks;
		std::queue<fs::path> QueuedFileChanges;

		std::mutex Mutex;
		U16 SleepDurationMilliseconds = 32;
		bool ShouldEndThread = false;
	};
}
