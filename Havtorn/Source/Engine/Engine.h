// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"

#include "Application/WindowHandler.h"

namespace Havtorn
{
	class CWindowHandler;
	class CThreadManager;
	class CGraphicsFramework;
	class GTimer;
	class CEditorManager;
	class CScene;
	class CRenderManager;
	class CParticleEmitterFactory;
	class CSpriteFactory;
	class CTextFactory;
	class CInputMapper;
	class CVFXMeshFactory;
	class CLineFactory;
	class CAudioManager;
	class CPhysXWrapper;
	class CSceneFactory;
	class CFileSystem;
	class CTextureBank;

	class GEngine
	{
		friend class CVFXMeshFactory;
		friend class CRenderManager;
		friend class CLineFactory;
		friend class CTextureBank;
		friend class CSceneFactory;

	public:
		GEngine();
		~GEngine();
		bool Init(const CWindowHandler::SWindowData& windowData);
		float BeginFrame();
		void Update();
		void RenderFrame();
		void EndFrame();

	private:
		static GEngine* GetInstance();
	public:
		static CWindowHandler* GetWindowHandler();
		static CFileSystem* GetFileSystem();
		static CTextureBank* GetTextureBank();
		static CInputMapper* GetInput();

		void InitWindowsImaging();
		void CrashWithScreenShot(std::wstring& subPath);

		void SetResolution(SVector2<F32> resolution);

		void ShowCursor(const bool& isInEditorMode = true);
		void HideCursor(const bool& isInEditorMode = false);

	private:
		static GEngine* Instance;

		CFileSystem* FileSystem = nullptr;
		CWindowHandler* WindowHandler = nullptr;
		CThreadManager* ThreadManager = nullptr;
		CGraphicsFramework* Framework = nullptr;
		CTextureBank* TextureBank = nullptr;
#ifdef _DEBUG
		CEditorManager* EditorManager = nullptr;
#endif
		CRenderManager* RenderManager = nullptr;
		GTimer* Timer = nullptr;
		CInputMapper* InputMapper = nullptr;

		CScene* Scene = nullptr;

		//CPhysXWrapper* PhysxWrapper;
		//CParticleEmitterFactory* ParticleFactory;
		//CVFXMeshFactory* VFXFactory;
		//CLineFactory* LineFactory;
		//CSpriteFactory* SpriteFactory;
		//CTextFactory* TextFactory;
		//CAudioManager* AudioManager;
	};
}