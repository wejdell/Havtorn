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
		for (int i = static_cast<int>(Processes.size() - 1); i >= 0; i--)
			delete Processes[i];
	}

	void CApplication::AddProcess(IProcess* process)
	{
		Processes.push_back(process);
	}

	void CApplication::Run()
	{
		Setup();

		// Don't do this if process count can change 
		const int processes = static_cast<int>(Processes.size() - 1);

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
			
			// Would we want to just know the type of the sub-process and then allocate & dea-llocate it here?
			// Or should the sub-process be pre-allocated, and then init & de-init?
			//		Could change Init to be Startup & Shutdown, destructor is kept for final part. Process destroys it's subprocess(es).
			// 
			// Here a Process should be able to push/pop a sub-process
			/*
			 	Find better name than "parentProcess"
			 
				while( AddSubProcessQueue > 0)
					(parentProcess, subProcess) = SubProcessQueue.pop
					parentProcess is given knowledge of index & start of execution
					startup subProcess
					add subProcess to Processes

				while( RemoveSubProcessQueue > 0)
					(parentProcess, subProcess) = SubProcessQueue.pop
					subProcessShutdown = find subProcess in Processes & remove it
					shutdown subProcessShutdown
					inform parentProcess of subProcessShutdown
			*/

			// Processes are run in reverse-order. Dependent to least dependent.

			for (int i = processes; i >= 0; i--)
				Processes[i]->BeginFrame();

			for (int i = processes; i >= 0; i--)
				Processes[i]->PreUpdate();

			for (int i = processes; i >= 0; i--)
				Processes[i]->Update();

			for (int i = processes; i >= 0; i--)
				Processes[i]->PostUpdate();

			for (int i = processes; i >= 0; i--)
				Processes[i]->EndFrame();
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
