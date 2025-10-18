// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <vector>
#include <string>

namespace Havtorn
{
	class IProcess;
	class CPlatformManager;

	class CApplication
	{
	public:
		CApplication();
		~CApplication();

		// Least-dependent-lowest-level processes should be added first.
		// Processes are initialized in order of addition, run and destructed in reverse-order.
		void AddProcess(IProcess* process);

		void Run(const std::string& commandLine);
		void Setup(CPlatformManager* platformManager);

	private:

	private:
		std::vector<IProcess*> Processes;
		bool IsRunning;
	};
}
