// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
//#include "Application/WindowHandler.h"

namespace Havtorn
{
	class GDebugDraw;
}

namespace Havtorn
{
	class CPlatformManager;
	//class CWindowHandler;
	class CThreadManager;
	class CGraphicsFramework;
	class CRenderManager;
	class GTime;
	class CWorld;
	class CInputMapper;
	class CAudioManager;
	class CFileSystem;
	class CTextureBank;
	class CSequencerSystem;
	class CEngineProcess;
	class IProcess;


	class GEngine
	{
		friend class CRenderManager;
		friend class CTextureBank;
		friend class CEditorProcess;
		friend class CImGuiProcess;
		friend class CGameProcess;
		//friend CWindowHandler;

	public:
		GEngine();
		~GEngine();
		bool Init(CPlatformManager* platformManager);
		float BeginFrame();
		void Update();
		void RenderFrame();
		void EndFrame();
		
		//static ENGINE_API CWindowHandler* GetWindowHandler();
		static ENGINE_API CFileSystem* GetFileSystem();
		static ENGINE_API CTextureBank* GetTextureBank();
		static ENGINE_API CThreadManager* GetThreadManager();
		static ENGINE_API CWorld* GetWorld();
		static ENGINE_API CInputMapper* GetInput();
		static ENGINE_API CGraphicsFramework* GetGraphicsFramework();
		
		void OnWindowResolutionChanged(SVector2<U16> newResolution);

	public:
		void CrashWithScreenShot(std::wstring& subPath);

		//void ShowCursor(const bool& isInEditorMode = true);
		//void HideCursor(const bool& isInEditorMode = false);

	private:
		static ENGINE_API GEngine* Instance;

		CFileSystem* FileSystem = nullptr;
		//CWindowHandler* WindowHandler = nullptr;
		CThreadManager* ThreadManager = nullptr;
		CGraphicsFramework* Framework = nullptr;
		CTextureBank* TextureBank = nullptr;
		CRenderManager* RenderManager = nullptr;
		GTime* Timer = nullptr;
		GDebugDraw* DebugDraw = nullptr;
		CInputMapper* InputMapper = nullptr;
		CWorld* World = nullptr;

		// TODO.NR: Figure out how to set up dependencies for Editor systems on Engine ECS Systems
		CSequencerSystem* SequencerSystem = nullptr;

		SVector2<U16> WindowResizeTarget = {};
	};
}
