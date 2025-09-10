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

bool TrySendToRunningInstance(const std::wstring& uri)
{
	HWND targetWindowHWND = FindWindowW(L"HavtornWindowClass", L"Havtorn Editor");
	if (!targetWindowHWND)
		return false;

	COPYDATASTRUCT copyDataStruct;
	copyDataStruct.dwData = 1;
	copyDataStruct.cbData = (DWORD)(uri.size() + 1) * sizeof(wchar_t);
	copyDataStruct.lpData = (PVOID)uri.c_str();
	//LRESULT result = 
	SendMessageW(targetWindowHWND, WM_COPYDATA, 0, (LPARAM)&copyDataStruct);
	return true;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	hInstance;
	hPrevInstance;
	lpCmdLine;
	nShowCmd;

#ifdef HV_DEEPLINK_ENALBED
	// Note.AS:
	// Overrides CurrentDirectory to be as if you started this application from the exe's location- which is not true when deeplink-starting this executable
	wchar_t wideExePath[255];
	GetModuleFileNameW(NULL, wideExePath, 255);
	std::filesystem::path exePath(wideExePath);
	std::filesystem::path exeDirectory = exePath.parent_path();
	SetCurrentDirectoryW(exeDirectory.c_str());

	std::wstring cmdLine = GetCommandLineW();
	if (TrySendToRunningInstance(cmdLine))
	{
		return 0;
	}
#endif



#ifdef USE_CONSOLE
	OpenConsole();
#endif
	CPlatformProcess* platformProcess = new CPlatformProcess(100, 100, 1280, 720);
	GUIProcess* guiProcess = new GUIProcess();
	CEngineProcess* engineProcess = new CEngineProcess();
	CGameProcess* gameProcess = new CGameProcess();
	CEditorProcess* editorProcess = new CEditorProcess();

	auto application = new CApplication();
	application->AddProcess(platformProcess);
	application->AddProcess(engineProcess);
	application->AddProcess(guiProcess);
	application->AddProcess(gameProcess);
	application->AddProcess(editorProcess);

	platformProcess->Init(nullptr);
	engineProcess->Init(platformProcess->PlatformManager);
	// TODO.NW: guiProcess init should handle InitGUI, need hold of the render backend somehow. maybe still move render backend to platform manager
	guiProcess->Init(platformProcess->PlatformManager);
	auto backend = engineProcess->GetRenderBackend();
	guiProcess->InitGUI(platformProcess->PlatformManager, backend.device, backend.context);
	gameProcess->Init(platformProcess->PlatformManager);
	editorProcess->Init(platformProcess->PlatformManager);
	//application->Setup(platformProcess->PlatformManager); //foreach -> process->Init();

	application->Run();

	delete application;

	SetForegroundWindow(GetConsoleWindow());

	system("pause");

#ifdef USE_CONSOLE
	CloseConsole();
#endif

	return 0;
}

#endif
