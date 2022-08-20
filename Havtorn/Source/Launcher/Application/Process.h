// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
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
	};
}