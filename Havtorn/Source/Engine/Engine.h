// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class GDebugDraw;
}

namespace Havtorn
{
	class CPlatformManager;
	class CRHI;
	class CThreadManager;
	class CRenderManager;
	class GTime;
	class CWorld;
	class CInputMapper;
	class UFileSystem;
	class CFileWatcher;
	class CAssetRegistry;
	class CSequencerSystem;
	class CEngineProcess;
	class IProcess;

	class GEngine
	{
		friend class CRenderManager;
		friend class CEditorProcess;
		friend class CGameProcess;

	public:
		GEngine();
		~GEngine();
		bool Init(CPlatformManager* platformManager);
		float BeginFrame();
		void Update();
		void RenderFrame();
		void EndFrame();
		
		static ENGINE_API CFileWatcher* GetFileWatcher();
		static ENGINE_API CThreadManager* GetThreadManager();
		static ENGINE_API CAssetRegistry* GetAssetRegistry();
		static ENGINE_API CWorld* GetWorld();
		static ENGINE_API CInputMapper* GetInput();
		
		void OnWindowResolutionChanged(SVector2<U16> newResolution);

	public:
		void CrashWithScreenShot(std::wstring& subPath);

	private:
		static ENGINE_API GEngine* Instance;

		CRHI* RHI = nullptr; // Not owned by engine

		// TODO.NW: Might as well make these unique ptrs
		CFileWatcher* FileWatcher = nullptr;
		CThreadManager* ThreadManager = nullptr;
		CRenderManager* RenderManager = nullptr;
		CAssetRegistry* AssetRegistry = nullptr;
		// TODO.NW: Change "G" and "U" standard to something common?
		GTime* Timer = nullptr;
		GDebugDraw* DebugDraw = nullptr;
		CInputMapper* InputMapper = nullptr;
		CWorld* World = nullptr;

		// TODO.NW: Figure out how to set up dependencies for Editor systems on Engine ECS Systems
		CSequencerSystem* SequencerSystem = nullptr;

		SVector2<U16> WindowResizeTarget = {};
	};
}
