// Copyright 2022 Team Havtorn. All Rights Reserved.

#include <Havtorn.h>
#include <iostream>

#include "Application/Application.h"
#include "../Engine/Application/EngineProcess.h"
#include "../Game/GameProcess.h"
#include "../Editor/EditorProcess.h"
#include "../Engine/Application/ImGuiProcess.h"

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

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	hInstance;
	hPrevInstance;
	lpCmdLine;
	nShowCmd;

#ifdef USE_CONSOLE
	OpenConsole();
#endif

	GLog::Init();

	CEngineProcess* engineProcess = new CEngineProcess(100, 100, 1280, 720);
	CImGuiProcess* imGuiProcess = new CImGuiProcess();
	CGameProcess* gameProcess = new CGameProcess();
	CEditorProcess* editorProcess = new CEditorProcess();

	auto application = new CApplication();
		application->AddProcess(engineProcess);
		application->AddProcess(imGuiProcess);
		application->AddProcess(gameProcess);
		application->AddProcess(editorProcess);

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
