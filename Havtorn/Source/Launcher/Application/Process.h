// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	// How to handle suspend?
	// Application should be able to suspend process? But how would it know?
	// Have process track it's own suspension?
	// have a flag?
	//		We may want to render part of process or run minor logic
	//		Suspend Logic [ PreUpdate, Update, PostUpdate ]
	//		Suspend Visual [BeginFrame, EndFrame ]
	//		=> Would however suggest a certain structure, neither good or bad. 
	//		
	//	Would prefer not having consant checks in runtime.
	//

	// Is no longer Interface? But inheritable class?
	class _declspec(dllexport) IProcess
	{
	public:
		IProcess() = default;
		virtual ~IProcess() = default;

		// Called once to see if Process can be run. Terminates Application if returns false.
		virtual bool Init() = 0;

		virtual void BeginFrame() = 0;
		virtual void PreUpdate() = 0;
		virtual void Update() = 0;
		virtual void PostUpdate() = 0;
		virtual void EndFrame() = 0;


		// Could make separate interface/class to accomplish this, since knowledge of CApplication is desired, indirectly (callbacks would be good)
		//		Something like MapSubProcess( 
		//										callback to application for adding to start queue,
		//										callback to application for adding to shutdown queue,
		//										callback for knowing startup
		//										callback for knowing shutdown
		//									)
		// 
		//		Able to map to string? Process would want to know what it is passing off. 
		//		I.e from Editor we want something that can just do : StartSubProcess("Game"), Suspend current Process
		// 
		// void StartSubProcess(string)
		// virutal void OnSubProcessStarted(string)
		// void ShutdownSubProcess(string)
		// virtual void OnShutdownSubProcess(string)

		// void QueueSubProcessExecution(IProcess parent, IProcess sub-process) {}
		// void QueueSubProcessShutdown(IProcess parent, IProcess sub-process {}
	};
}