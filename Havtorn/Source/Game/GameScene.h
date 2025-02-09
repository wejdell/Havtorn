// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include <Scene\Scene.h>

namespace Havtorn
{
    class CGameScene : public CScene
    {
    public:
		HAVTORN_EXPORT virtual bool Init(CRenderManager* renderManager, const std::string& sceneName) override;

		HAVTORN_EXPORT virtual bool Init3DDemoScene(CRenderManager* renderManager) override;
		HAVTORN_EXPORT virtual bool Init2DDemoScene(CRenderManager* renderManager) override;

		HAVTORN_EXPORT virtual [[nodiscard]] U32 GetSize() const override;
		HAVTORN_EXPORT virtual void Serialize(char* toData, U64& pointerPosition) const override;
		HAVTORN_EXPORT virtual void Deserialize(const char* fromData, U64& pointerPosition, CAssetRegistry* assetRegistry) override;
    };
}

