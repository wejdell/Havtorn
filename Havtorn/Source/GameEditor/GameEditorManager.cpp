// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "GameEditorManager.h"

namespace Havtorn
{
    bool CGameEditorManager::Init(CPlatformManager* platformManager, CRenderManager* renderManager)
    {
        bool returnValue = CEditorManager::Init(platformManager, renderManager);
        if (!returnValue)
            return false;

        return true;
    }
}
