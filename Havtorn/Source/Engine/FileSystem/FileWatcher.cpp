// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "FileWatcher.h"

#include <fstream>
#include <algorithm>
#include <ranges>

#include "Engine.h"
#include "Threading/ThreadManager.h"

namespace fs = std::filesystem;

namespace Havtorn
{
	CFileWatcher::~CFileWatcher()
	{
	}

	bool CFileWatcher::Init(CThreadManager* threadManager)
	{
		if (!threadManager)
			return false;
 
		threadManager->ScheduleRepeatingTask(std::bind(&CFileWatcher::CheckForFileUpdates, this), SleepDurationMilliseconds);

		return true;
	}

	void CFileWatcher::FlushChanges()
	{
		std::lock_guard<std::mutex> lock(Mutex);
		while (!QueuedFileChanges.empty())
		{
			const fs::path& filePath = QueuedFileChanges.front();
			if (!StoredCallbacks.contains(filePath))
			{
				QueuedFileChanges.pop();
				continue;
			}

			const std::vector<SFileChangeCallback>& callbacks = StoredCallbacks[filePath];
			for (const SFileChangeCallback& callback : callbacks)
				callback.Function(filePath.string());
			
			QueuedFileChanges.pop();
		}
	}

	U64 GetFileTimestamp(const fs::path& filePath)
	{
		if (std::filesystem::exists(filePath) == false)
			return 0;

		return std::filesystem::last_write_time(filePath).time_since_epoch().count();
	}

	void CFileWatcher::CheckForFileUpdates()
	{		
		std::lock_guard<std::mutex> lock(Mutex);
		for (const auto& [path, currentTimestamp] : WatchedFiles)
		{
			const U64 latestTimeStamp = GetFileTimestamp(path);
			if (latestTimeStamp > currentTimestamp)
			{
				QueuedFileChanges.push(path);
				WatchedFiles[path] = latestTimeStamp;
			}
		}	
	}

	bool CFileWatcher::WatchFileChange(const std::string& filePath, SFileChangeCallback callback)
	{
		// TODO.NW: Maybe add feature for turning on and off file watcher? Shouldn't be active while playing.
		// Could add atomic bool that we send to ScheduleRepeatingTask in CFileWatcher::Init, and expose a function
		// to toggle that, setting the bool to true when toggling watching off, and calling ScheduleRepeatingTask 
		// again when toggling watching on

		const fs::path newPath = filePath.c_str();

		if (!std::filesystem::exists(newPath))
			return false;

		std::lock_guard<std::mutex> lock(Mutex);
		StoredCallbacks[newPath].push_back(callback);

		if (!WatchedFiles.contains(newPath))
			WatchedFiles.emplace(newPath, GetFileTimestamp(newPath));

		return true;
	}

	void CFileWatcher::StopWatchFileChange(const std::string& filePath, const U64 callbackHandle)
	{
		const fs::path existingPath = filePath.c_str();

		if (!std::filesystem::exists(existingPath))
			return;

		if (!StoredCallbacks.contains(existingPath))
			return;

		std::lock_guard<std::mutex> lock(Mutex);
		std::vector<SFileChangeCallback>& callbackContainer = StoredCallbacks.at(existingPath);

		auto it = std::ranges::find(callbackContainer, callbackHandle, &SFileChangeCallback::Handle);
		if (it == callbackContainer.end())
			return;

		callbackContainer.erase(it);

		if (!callbackContainer.empty())
			return;

		StoredCallbacks.erase(existingPath);
		WatchedFiles.erase(existingPath);
	}
}
