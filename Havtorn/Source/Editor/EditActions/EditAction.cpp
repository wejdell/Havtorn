// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "EditAction.h"

namespace Havtorn
{
    SEditAction::SEditAction(const SMetaCommand& command, const std::string& compactName)
        : Command(command)
        , CompactName(compactName)
    {
    }

    void SEditAction::ResolveAction(CEditorManager* /*manager*/, const bool /*inverted*/)
    {
    }

    std::string SEditAction::ResolveCompactName(const SMetaCommand& command)
    {
        // TODO.NW: Make readable names for all handled commands

        constexpr U64 parserNameLength = 12;
        return command.Name.substr(parserNameLength);
    }
}
