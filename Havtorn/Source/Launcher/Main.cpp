// Copyright 2022 Team Havtorn. All Rights Reserved.

#include <Havtorn.h>
#include <CommandLine.h>

#include "Application/Application.h"
#include <../Platform/PlatformProcess.h>
#include <../Engine/Application/EngineProcess.h>
#include <../Game/GameProcess.h>
#include <../Editor/EditorProcess.h>
#include <../GUI/GUIProcess.h>

#include <../Platform/PlatformManager.h>

#include <iostream>
#include <filesystem>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

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

bool TrySendToRunningInstance(const std::string& uri)
{
	HWND targetWindowHandle = FindWindowA("SDL_app", "Havtorn Editor");
	if (!targetWindowHandle)
		return false;

	COPYDATASTRUCT copyDataStruct;
	copyDataStruct.dwData = 1;
	copyDataStruct.cbData = (DWORD)(uri.size() + 1) * sizeof(char);
	copyDataStruct.lpData = (PVOID)uri.c_str();
	
	SendMessageA(targetWindowHandle, WM_COPYDATA, 0, (LPARAM)&copyDataStruct);
	return true;
}
#endif

using namespace Havtorn;

I32 main(I32 argc, char* argv[])
{
	std::string commandLine;
	for (I32 index = 0; index < argc; index++)
	{
		commandLine.append(argv[index]);
		commandLine.append(" ");
	}
	UCommandLine::Parse(commandLine);

#if defined(HV_DEEPLINK_ENABLED) && defined(HV_PLATFORM_WINDOWS)
	// Note.AS:
	// Overrides CurrentDirectory to be as if you started this application from the exe's location- which is not true when deeplink-starting this executable
	UFileSystem::SetWorkingPath(UGeneralUtils::ExtractParentDirectoryFromPath(UFileSystem::GetExecutableRootPath()));
	if (TrySendToRunningInstance(commandLine))
	{
		return 0;
	}
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

#ifdef USE_CONSOLE
	OpenConsole();
#endif

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
	application->Run();
	delete application;

#ifdef USE_CONSOLE
	CloseConsole();
#endif

	return 0;
}

