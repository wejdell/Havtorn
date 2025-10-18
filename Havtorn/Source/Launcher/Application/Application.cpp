// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Application.h"
#include "Process.h"

#include <CoreTypes.h>

#include <WindowsInclude.h>

namespace Havtorn
{
	CApplication::CApplication() 
	{
		IsRunning = true;
	}

	CApplication::~CApplication() 
	{
		for (I16 i = STATIC_I16(Processes.size() - 1); i >= 0; i--)
			delete Processes[i];
	}

	void CApplication::AddProcess(IProcess* process)
	{
		Processes.push_back(process);
	}

	void CApplication::Run(const std::string& command)
	{
		for (auto process : Processes)
			process->OnApplicationReady(command);

		const I16 numberOfProcesses = STATIC_I16(Processes.size() - 1);

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

			// Processes are run in reverse-order. Dependent to least dependent.

			for (I16 i = numberOfProcesses; i >= 0; i--)
				Processes[i]->BeginFrame();

			for (I16 i = numberOfProcesses; i >= 0; i--)
				Processes[i]->PreUpdate();

			for (I16 i = numberOfProcesses; i >= 0; i--)
				Processes[i]->Update();

			for (I16 i = numberOfProcesses; i >= 0; i--)
				Processes[i]->PostUpdate();

			for (I16 i = numberOfProcesses; i >= 0; i--)
				Processes[i]->EndFrame();
		}
	}

	void CApplication::Setup(CPlatformManager* platformManager)
	{
		Processes.shrink_to_fit();

		for (auto process : Processes)
		{
			if (!process->Init(platformManager))
			{
				IsRunning = false;
				break;
			}
		}
	}
}