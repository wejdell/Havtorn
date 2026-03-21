// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

union SDL_Event;

namespace Havtorn
{
	class CInput;
	class CPlatformManager;

	class CInputPoller
	{
		friend class GEngine;

	private:
		CInputPoller();
		~CInputPoller();

		CInputPoller(const CInputPoller&) = delete;
		CInputPoller(CInputPoller&&) = delete;
		CInputPoller operator=(const CInputPoller&) = delete;
		CInputPoller operator=(CInputPoller&&) = delete;

	public:
		bool Init(CPlatformManager* platformManager);
		void Update();
		void ProcessEvent(const SDL_Event* event);

		//[[nodiscard]] ENGINE_API 
		//[[nodiscard]] ENGINE_API

	private:
		void UpdateKeyInput();
		void UpdateAxisInput();

		CInput* Input = nullptr;
	};
}