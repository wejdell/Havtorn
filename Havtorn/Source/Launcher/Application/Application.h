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

		void AddProcess(IProcess* process);

		void Run();

	private:
		void Setup();

	private:
		std::vector<IProcess*> Processes;
		bool IsRunning;
	};
}
