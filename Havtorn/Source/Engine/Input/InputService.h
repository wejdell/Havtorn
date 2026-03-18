// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CInput;
	class CPlatformManager;

	class CInputService
	{
		friend class GEngine;

	private:
		CInputService();
		~CInputService();

		CInputService(const CInputService&) = delete;
		CInputService(CInputService&&) = delete;
		CInputService operator=(const CInputService&) = delete;
		CInputService operator=(CInputService&&) = delete;

	public:
		void Update();

		//[[nodiscard]] ENGINE_API 
		//[[nodiscard]] ENGINE_API

	private:
		void UpdateKeyInput();
		void UpdateAxisInput();

		CInput* Input = nullptr;
	};
}