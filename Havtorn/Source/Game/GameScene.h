// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include <Scene\Scene.h>

namespace Havtorn
{
    class CGameScene : public CScene
    {
    public:
		__declspec(dllexport) virtual bool Init3DDemoScene(CRenderManager* renderManager) override;
		__declspec(dllexport) virtual bool Init2DDemoScene(CRenderManager* renderManager) override;

		__declspec(dllexport) virtual [[nodiscard]] U32 GetSize() const override;
		__declspec(dllexport) virtual void Serialize(char* toData, U64& pointerPosition) const override;
		__declspec(dllexport) virtual void Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry) override;
    };
}

