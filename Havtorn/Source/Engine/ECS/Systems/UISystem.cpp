// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "UISystem.h"
#include "Engine.h"

#include "Input/InputMapper.h"

#include "ECS/Components/UICanvasComponent.h"
#include "ECS/Components/Transform2DComponent.h"

#include "Scene/World.h"

#include <PlatformManager.h>

namespace Havtorn 
{
	CUISystem::CUISystem(CPlatformManager* platformManager)
		: ISystem()
	{
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::MousePositionHorizontal).AddMember(this, &CUISystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::MousePositionVertical).AddMember(this, &CUISystem::HandleAxisInput);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::PickEditorEntity).AddMember(this, &CUISystem::HandleMouseInput);

		PlatformManager = platformManager;

		PlayGameFunction = std::bind(&CUISystem::PlayGame, this);
		BindEvaluateFunction(PlayGameFunction, "CUISystem::PlayGame");
	}

	CUISystem::~CUISystem()
	{
	}

	void CUISystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		HV_LOG_INFO("Mouse pos: %s", MousePosition.ToString().c_str());

		if (GEngine::GetWorld()->GetWorldPlayState() != EWorldPlayState::Playing)
			return;

		for (Ptr<CScene>& scene : scenes)
		{
			for (SUICanvasComponent* canvas : scene->GetComponents<SUICanvasComponent>())
			{
				if (!FocusedCanvas.IsValid())
					FocusedCanvas = canvas->Owner;

				if (canvas->Owner != FocusedCanvas)
					continue;

				STransform2DComponent* transformComp = scene->GetComponent<STransform2DComponent>(canvas);
				if (canvas == nullptr || transformComp == nullptr)
					continue;

				SUIElement& element = canvas->Elements[0];
				const F32 left = (element.LocalPosition.X + element.CollisionRect.X);
				const F32 right = (element.LocalPosition.X + element.CollisionRect.Z);
				const F32 bottom = (element.LocalPosition.Y + element.CollisionRect.Y);
				const F32 top = (element.LocalPosition.Y + element.CollisionRect.W);
				bool isInRect = UMath::IsWithin(MousePosition.X, transformComp->Position.X + left, transformComp->Position.X + right);
				isInRect = isInRect && UMath::IsWithin(MousePosition.Y, transformComp->Position.Y + bottom, transformComp->Position.Y + top);

				EUIElementState newState = (isInRect && !IsMouseClicked) ? EUIElementState::Hovered : EUIElementState::Idle;

				if ((element.State == EUIElementState::Hovered || element.State == EUIElementState::Active) && isInRect && IsMouseClicked)
					newState = EUIElementState::Active;

				// Release on element to click
				if (element.State == EUIElementState::Active && newState == EUIElementState::Hovered)
				{
					HV_LOG_INFO("Clicked");

					if (element.BindingType == EUIBindingType::GenericFunction && FunctionMap.contains(element.BoundData))
						FunctionMap[element.BoundData]();
					else if (element.BindingType == EUIBindingType::OtherCanvas && scene->HasEntity(element.BoundData))
						FocusedCanvas = SEntity{ element.BoundData };
				}

				element.State = newState;
			}
		}
	}

	void CUISystem::BindEvaluateFunction(std::function<void()>& function, const std::string& classAndFunctionName)
	{
		U64 id = std::hash<std::string>{}(classAndFunctionName);

		if (FunctionMap.contains(id))
		{
			FunctionMap.erase(id);
			IdentifierMap.erase(id);
		}

		FunctionMap.emplace(id, function);
		IdentifierMap.emplace(id, classAndFunctionName);
	}

	std::string CUISystem::GetFunctionName(const U64 boundFunctionHash)
	{
		if (IdentifierMap.contains(boundFunctionHash))
			return IdentifierMap.at(boundFunctionHash);

		return "Function Not Found";
	}

	void CUISystem::PlayGame()
	{
	}

	void CUISystem::HandleAxisInput(const SInputAxisPayload payload)
	{
		SVector2<U16> resolution = PlatformManager->GetResolution();

		if (payload.Event == EInputAxisEvent::MousePositionHorizontal)
			MousePosition.X = payload.AxisValue / STATIC_F32(resolution.X);

		if (payload.Event == EInputAxisEvent::MousePositionVertical)
			MousePosition.Y = 1.0f - (payload.AxisValue / STATIC_F32(resolution.Y));
	}

	void CUISystem::HandleMouseInput(const SInputActionPayload payload)
	{
		// TODO.NW: Make mouse click event?
		if (payload.IsPressed || payload.IsHeld)
			IsMouseClicked = true;

		if (payload.IsReleased)
			IsMouseClicked = false;
	}
}
