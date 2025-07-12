// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class GDebugDraw;
}

namespace Havtorn
{
	class CPlatformManager;
	class CThreadManager;
	class CGraphicsFramework;
	class CRenderManager;
	class GTime;
	class CWorld;
	class CInputMapper;
	class CAudioManager;
	class CFileSystem;
	class CFileWatcher;
	class CTextureBank;
	class CSequencerSystem;
	class CEngineProcess;
	class IProcess;

	class GEngine
	{
		friend class CRenderManager;
		friend class CTextureBank;
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
		
		static ENGINE_API CFileSystem* GetFileSystem();
		static ENGINE_API CFileWatcher* GetFileWatcher();
		static ENGINE_API CTextureBank* GetTextureBank();
		static ENGINE_API CThreadManager* GetThreadManager();
		static ENGINE_API CWorld* GetWorld();
		static ENGINE_API CInputMapper* GetInput();
		static ENGINE_API CGraphicsFramework* GetGraphicsFramework();
		
		void OnWindowResolutionChanged(SVector2<U16> newResolution);

	public:
		void CrashWithScreenShot(std::wstring& subPath);

	private:
		static ENGINE_API GEngine* Instance;

		CFileSystem* FileSystem = nullptr;
		CFileWatcher* FileWatcher = nullptr;
		CThreadManager* ThreadManager = nullptr;
		CGraphicsFramework* Framework = nullptr;
		CTextureBank* TextureBank = nullptr;
		CRenderManager* RenderManager = nullptr;
		GTime* Timer = nullptr;
		GDebugDraw* DebugDraw = nullptr;
		CInputMapper* InputMapper = nullptr;
		CWorld* World = nullptr;

		// TODO.NW: Figure out how to set up dependencies for Editor systems on Engine ECS Systems
		CSequencerSystem* SequencerSystem = nullptr;

		SVector2<U16> WindowResizeTarget = {};
	};
}
