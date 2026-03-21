// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "InputPoller.h"
#include "PlatformManager.h"
#include "Input/Input.h"


namespace Havtorn
{
	CInputPoller::CInputPoller()
		: Input(CInput::GetInstance())
	{

	}

	CInputPoller::~CInputPoller()
	{

	}

	bool CInputPoller::Init(CPlatformManager* platformManager)
	{
		if (platformManager == nullptr)
			return false;

		platformManager->OnProcessEvent.AddMember(this, &CInputPoller::ProcessEvent);
		return true;
	}

	void CInputPoller::Update()
	{

	}

	void CInputPoller::ProcessEvent(const SDL_Event* event)
	{
		event;
	}

	void CInputPoller::UpdateKeyInput()
	{

	}

	void CInputPoller::UpdateAxisInput()
	{

	}
}
