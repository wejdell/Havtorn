// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <string>

namespace Havtorn
{
	class CPlatformManager;

	class _declspec(dllexport) IProcess
	{
	public:
		IProcess() = default;
		virtual ~IProcess() = default;

		// Called once to see if Process can be run. Terminates Application if returns false.
		virtual bool Init(CPlatformManager* platformManager) = 0;
		virtual void OnApplicationReady(const std::string& /*commandLine*/) {};

		virtual void BeginFrame() {};
		virtual void PreUpdate() {};
		virtual void Update() {};
		virtual void PostUpdate() {};
		virtual void EndFrame() {};
	};
}