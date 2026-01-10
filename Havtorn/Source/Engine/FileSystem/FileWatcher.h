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

	typedef std::function<void(const std::string&)> CFileChangeCallback;
	
	class CFileWatcher
	{
	public:
		CFileWatcher() = default;
		~CFileWatcher();

		bool Init(CThreadManager* threadManager);

		ENGINE_API bool WatchFileChange(const std::string& filePath, CFileChangeCallback callback); 
		ENGINE_API void StopWatchFileChange(const std::string& filePath, CFileChangeCallback callback);
		
		void FlushChanges(); 

	private:
		void UpdateChanges(); 

		std::map<fs::path, U64> WatchedFiles;
		
		std::map<fs::path, std::vector<CFileChangeCallback>> StoredCallbacks;
		std::queue<fs::path> QueuedFileChanges;

		std::mutex Mutex;
		U16 SleepDurationMilliseconds = 32;
		bool ShouldEndThread = false;
	};
}
