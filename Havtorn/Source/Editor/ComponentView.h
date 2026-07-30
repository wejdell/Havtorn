// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include <Core.h>

namespace Havtorn
{
	struct SEntity;
	class CScene;
	class CEditorManager;

	struct EDITOR_API SComponentView
	{
		friend class CEditorManager;

		virtual void View(const SEntity& /*entityOwner*/, CScene* /*scene*/) const {};
		virtual const char* GetComponentName() const { return "Component"; };
		virtual U8 GetSortingPriority() const { return UMath::MaxU8; };
		U64 GetRuntimeHash() const { return RuntimeHash; }

	protected:
		CEditorManager* Manager = nullptr;

	private:
		U64 RuntimeHash = 0;
	};
}
