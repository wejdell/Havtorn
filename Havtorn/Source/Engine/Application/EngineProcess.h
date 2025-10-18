// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <../Launcher/Application/Process.h>


struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{

	struct ENGINE_API SEngineRenderBackend
	{
		HWND hwnd;
		ID3D11Device* device;
		ID3D11DeviceContext* context;
	};

	class ENGINE_API CEngineProcess : public IProcess
	{
	public:
		CEngineProcess();
		~CEngineProcess() override;

		bool Init(CPlatformManager* platformManager) override;

		void BeginFrame() override;
		void Update() override;
		void PostUpdate() override;
		void EndFrame() override;

		SEngineRenderBackend GetRenderBackend();

	private:
		class GEngine* Engine = nullptr;
	};
}
