// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "EditAction.h"

namespace Havtorn
{
    SEditAction::SEditAction(const SMetaCommand& command)
        : Command(command)
    {
    }

    void SEditAction::ResolveAction(CEditorManager* /*manager*/, const bool /*inverted*/)
    {
    }

    std::string SEditAction::GetCompactName()
    {
        // TODO.NW: Make readable names for all handled commands

        constexpr U64 parserNameLength = 12;
        return Command.Name.substr(parserNameLength);
    }
}
