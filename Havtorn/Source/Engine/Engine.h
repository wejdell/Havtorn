// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Application/WindowHandler.h"

namespace Havtorn
{
	class CWindowHandler;
	class CThreadManager;
	class CDirextXFramework;
	class CModelFactory;
	class CCameraFactory;
	class CLightFactory;
	class CGraphicsFramework;
	class GTimer;
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
		friend class CEditorProcess;

	public:
		GEngine();
		~GEngine();
		bool Init(const CWindowHandler::SWindowData& windowData);
		float BeginFrame();
		void Update();
		void RenderFrame();
		void EndFrame();
		
		HAVTORN_API CWindowHandler* GetWindowHandler();
		HAVTORN_API CFileSystem* GetFileSystem();
		HAVTORN_API CMaterialHandler* GetMaterialHandler();
		HAVTORN_API CTextureBank* GetTextureBank();
		HAVTORN_API CThreadManager* GetThreadManager();
		
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