// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include <functional>
#include <windows.h>
#include <thread>
#include <mutex>
#include <filesystem>

namespace fs = std::filesystem;

namespace Havtorn
{
	class CThreadManager;

	typedef std::function<void(const std::string&)> FileChangeCallback;
	
	class CFileWatcher
	{
	public:
		CFileWatcher() = default;
		~CFileWatcher();

		bool Init(CThreadManager* /*threadManager*/);

		/* Will check the file for includes and add them as well*/
		bool WatchFileChange(const std::string& fileName, FileChangeCallback callback);
		void FlushChanges();

	private:
		void UpdateChanges();
		void CheckFileChanges(const fs::path& filePath, const U64 timeStampLastChanged);
		void OnFileChange(const fs::path& filePath);

		std::thread* Thread = nullptr;

		std::vector<fs::path> FilesChangedThreaded;
		std::vector<fs::path> FilesChanged;
		std::map<std::string, std::vector<FileChangeCallback>> Callbacks;
		std::map<fs::path, I64> ThreadedFilesToWatch;

		std::mutex Mutex;
		std::mutex AddNewFolderMutex;
		U64 PeriodMilliseconds = 32;
		bool ShouldEndThread = false;
		bool ThreadIsDone = false;
	};
}
