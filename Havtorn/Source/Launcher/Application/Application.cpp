// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Application.h"
#include "Process.h"

#include <Windows.h>

namespace Havtorn
{
	CApplication::CApplication() 
	{
		IsRunning = true;
	}

	CApplication::~CApplication() 
	{
		for (int i = 0; i < Processes.size(); i++)
		{
			delete Processes[i];
		}
	}

	void CApplication::AddProcess(IProcess* process)
	{
		Processes.push_back(process);
	}

	void CApplication::Run()
	{
		Setup();

		MSG windowMessage = { 0 };
		while (IsRunning)
		{
			while (PeekMessage(&windowMessage, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&windowMessage);
				DispatchMessage(&windowMessage);

				if (windowMessage.message == WM_QUIT)
				{
					IsRunning = false;
					break;
				}
			}

			for (const auto& process : Processes)
				process->BeginFrame();

			for (const auto& process : Processes)
				process->PreUpdate();

			for (const auto& process : Processes)
				process->Update();

			for (const auto& process : Processes)
				process->PostUpdate();

			for (const auto& process : Processes)
				process->EndFrame();
		}
	}

	void CApplication::Setup()
	{
		Processes.shrink_to_fit();

		for (int i = 0; i < Processes.size(); i++)
		{
			if (!Processes[i]->Init())
			{
				IsRunning = false;
				break;
			}
		}
	}
}
