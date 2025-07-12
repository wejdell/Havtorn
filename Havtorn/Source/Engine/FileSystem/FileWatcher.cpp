// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "FileWatcher.h"

#include <fstream>
#include <algorithm>

//#include "Threading/ThreadManager.h"

namespace fs = std::filesystem;

namespace Havtorn
{
	CFileWatcher::~CFileWatcher()
	{
		ShouldEndThread = true;
		if (Thread)
		{
			// TODO.NW: Figure out if there's a nicer way to kill the thread at the end?
			while (!ThreadIsDone)
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(1));
			}
			Thread->join();
			delete Thread;
		}
	}

	bool CFileWatcher::Init(CThreadManager* /*threadManager*/)
	{
		//if (!threadManager)
		//	return false;

		// TODO.NW: Would be nice to figure out if we can let the thread manager manage this thread as well
		//threadManager->PushJob(std::bind(&CFileWatcher::UpdateChanges, this, std::placeholders::_1));

		return true;
	}

	void CFileWatcher::FlushChanges()
	{
		// TODO.NW: Maybe add feature for turning on and off file watcher? Should it be active while playing? 
		// Could be a nice QOL feature to update assets while playing
		if (false/*!UsingFileWatcher*/ || !Thread)
			return;
		
		std::lock_guard<std::mutex> guard(Mutex);

		FilesChanged.swap(FilesChangedThreaded);

		for (const fs::path& path : FilesChanged)
		{
			std::string comparableIndexString = path.string();
			std::replace(comparableIndexString.begin(), comparableIndexString.end(), '\\', '#');
			std::replace(comparableIndexString.begin(), comparableIndexString.end(), '/', '#');
			std::vector<FileChangeCallback> callbacks = Callbacks[comparableIndexString];
			
			for (U64 i = 0; i < callbacks.size(); i++)
			{
				if (callbacks[i])
					callbacks[i](path.string());
			}
		}

		FilesChanged.clear();
	}

	void CFileWatcher::UpdateChanges()
	{
		while (!ShouldEndThread)
		{
			Mutex.lock();
			AddNewFolderMutex.lock();

			for (const auto& iter : ThreadedFilesToWatch)
			{
				CheckFileChanges(iter.first, iter.second);
			}

			Mutex.unlock();
			AddNewFolderMutex.unlock();

			std::this_thread::sleep_for(std::chrono::milliseconds(PeriodMilliseconds));
		}
		ThreadIsDone = true;
	}

	U64 GetFileTimeStamp(const fs::path& filePath)
	{
		if (std::filesystem::exists(filePath) == false)
			return 0;

		return std::filesystem::last_write_time(filePath).time_since_epoch().count();
	}

	void CFileWatcher::CheckFileChanges(const fs::path& filePath, const U64 timeStampLastChanged)
	{
		U64 latestTimeStamp = GetFileTimeStamp(filePath);
		if (latestTimeStamp > timeStampLastChanged)
		{
			OnFileChange(filePath);
			ThreadedFilesToWatch[filePath] = latestTimeStamp;
		}
	}

	void CFileWatcher::OnFileChange(const fs::path& filePath)
	{
		for (U64 i = 0; i < FilesChangedThreaded.size(); i++)
		{
			if (FilesChangedThreaded[i].compare(filePath) == 0)
				return;
		}

		FilesChangedThreaded.push_back(filePath);
	}

	bool CFileWatcher::WatchFileChange(const std::string& filePath, FileChangeCallback functionToCallOnChange)
	{
		// TODO.NW: Maybe add feature for turning on and off file watcher? Shouldn't be active while playing
		if (false/*!UsingFileWatcher*/)
			return false;

		if (!std::filesystem::exists(filePath.c_str()))
			return false;
		
		std::string comparableIndexString = filePath;
		std::replace(comparableIndexString.begin(), comparableIndexString.end(), '\\', '#');
		std::replace(comparableIndexString.begin(), comparableIndexString.end(), '/', '#');

		Callbacks[comparableIndexString].push_back(functionToCallOnChange);

		U64 timeStampChanged = GetFileTimeStamp(filePath.c_str());
		if (Thread)
		{
			AddNewFolderMutex.lock();
			ThreadedFilesToWatch[filePath] = timeStampChanged;
			AddNewFolderMutex.unlock();
		}
		else
		{
			ThreadedFilesToWatch[filePath] = timeStampChanged;
			Thread = new std::thread(&CFileWatcher::UpdateChanges, this);
		}

		return true;
	}
}
