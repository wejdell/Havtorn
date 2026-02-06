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

	void CApplication::Run()
	{
		for (auto process : Processes)
			process->OnApplicationReady();

		const I16 numberOfProcesses = STATIC_I16(Processes.size() - 1);

		while (IsRunning)
		{
			// Processes are run in reverse-order here. Dependent to least dependent.

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

			for (I16 i = numberOfProcesses; i >= 0; i--)
			{
				if (!Processes[i]->ShouldRun())
				{
					IsRunning = false;
					break;
				}
			}
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