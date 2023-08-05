// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <vector>

namespace Havtorn
{
	class IProcess;

	class CApplication
	{
	public:
		CApplication();
		~CApplication();

		// Least-dependent-lowest-level processes should be added first.
		// Processes are initialized in order of addition, run and destructed in reverse-order.
		void AddProcess(IProcess* process);

		void Run();

	private:
		void Setup();

	private:
		std::vector<IProcess*> Processes;
		bool IsRunning;
	};
}
