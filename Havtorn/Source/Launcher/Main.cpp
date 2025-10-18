// Copyright 2022 Team Havtorn. All Rights Reserved.

#include <Havtorn.h>
#include <iostream>

#include "Application/Application.h"
#include <../Platform/PlatformProcess.h>
#include <../Engine/Application/EngineProcess.h>
#include <../Game/GameProcess.h>
#include <../Editor/EditorProcess.h>
#include <../GUI/GUIProcess.h>
#include <filesystem>

#ifdef HV_PLATFORM_WINDOWS

#pragma region Console

#ifdef _DEBUG
#define USE_CONSOLE
#endif


void OpenConsole()
{
	::AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
	setvbuf(stdin, NULL, _IONBF, NULL);
	setvbuf(stdout, NULL, _IONBF, NULL);
	setvbuf(stderr, NULL, _IONBF, NULL);
}

void CloseConsole()
{
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
}

#pragma endregion

using namespace Havtorn;

bool TrySendToRunningInstance(const std::string& uri)
{
	HWND targetWindowHWND = FindWindowW(L"HavtornWindow", L"Havtorn Editor");
	if (!targetWindowHWND)
		return false;

	COPYDATASTRUCT copyDataStruct;
	copyDataStruct.dwData = 1;
	copyDataStruct.cbData = (DWORD)(uri.size() + 1) * sizeof(char);
	copyDataStruct.lpData = (PVOID)uri.c_str();
	
	SendMessageA(targetWindowHWND, WM_COPYDATA, 0, (LPARAM)&copyDataStruct);
	return true;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	hInstance;
	hPrevInstance;
	lpCmdLine;
	nShowCmd;

#ifdef HV_DEEPLINK_ENABLED
	// Note.AS:
	// Overrides CurrentDirectory to be as if you started this application from the exe's location- which is not true when deeplink-starting this executable
	UFileSystem::SetWorkingPath(UGeneralUtils::ExtractParentDirectoryFromPath(UFileSystem::GetExecutableRootPath()));

	// TODO.NW: Add better command line parsing
	std::string cmdLine = GetCommandLineA();
	if (TrySendToRunningInstance(cmdLine))
	{
		return 0;
	}
#endif

#ifdef USE_CONSOLE
	OpenConsole();
#endif

	CPlatformProcess* platformProcess = new CPlatformProcess(100, 100, 1280, 720);
	CEngineProcess* engineProcess = new CEngineProcess();
	CGameProcess* gameProcess = new CGameProcess();

#ifdef HV_EDITOR_BUILD
	GUIProcess* guiProcess = new GUIProcess();
	CEditorProcess* editorProcess = new CEditorProcess();
#endif

	auto application = new CApplication();
	application->AddProcess(platformProcess);
	application->AddProcess(engineProcess);
#ifdef HV_EDITOR_BUILD
	application->AddProcess(guiProcess);
#endif
	application->AddProcess(gameProcess);
#ifdef HV_EDITOR_BUILD
	application->AddProcess(editorProcess);
#endif

	platformProcess->Init(nullptr);
	engineProcess->Init(platformProcess->PlatformManager);
	
#ifdef HV_EDITOR_BUILD
	// TODO.NW: guiProcess init should handle InitGUI, need hold of the render backend somehow. maybe still move render backend to platform manager
	guiProcess->Init(platformProcess->PlatformManager);
	auto backend = engineProcess->GetRenderBackend();
	guiProcess->InitGUI(platformProcess->PlatformManager, backend.device, backend.context);
#endif
	gameProcess->Init(platformProcess->PlatformManager);
#ifdef HV_EDITOR_BUILD	
	editorProcess->Init(platformProcess->PlatformManager);
#endif

	//application->Setup(platformProcess->PlatformManager); //foreach -> process->Init();
	application->Run(cmdLine);
	delete application;

	SetForegroundWindow(GetConsoleWindow());

#ifdef USE_CONSOLE
	CloseConsole();
#endif

	return 0;
}
#endif
