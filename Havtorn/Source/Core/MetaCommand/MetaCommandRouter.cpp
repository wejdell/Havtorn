// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "MetaCommandRouter.h"

namespace Havtorn
{
    UMetaCommandRouter* UMetaCommandRouter::Instance = nullptr;

    void UMetaCommandRouter::Push(const SMetaCommand& command)
    {   
        if (!Instance->Parsers.contains(command.Parser))
        {
            HV_LOG_WARN("MetaCommandRouter could not resolve the intended parser of command: '%s'", command.Name.c_str());
            return;
        }

        // TODO.NW: Add optional logging toggling
        //HV_LOG_TRACE("MetaCommandRouter: pushed command '%s'", command.Name.c_str());
        Instance->Parsers.at(command.Parser)->Parse(command);
    }

    void UMetaCommandRouter::AddParser(IMetaCommandParser* parser)
    {
        if (parser->ParserID == 0)
        {
            HV_LOG_WARN("Tried to add MetaCommandParser with invalid ID 0.");
            return; 
        }

        if (!Instance)
            Instance = new UMetaCommandRouter();

        Instance->Parsers.emplace(parser->ParserID, parser);
    }

    void UMetaCommandRouter::RemoveParser(IMetaCommandParser* parser)
    {
        if (parser->ParserID == 0)
        {
            HV_LOG_WARN("Tried to remove MetaCommandParser with invalid ID 0.");
            return; 
        }

        if (!Instance)
            return;

        Instance->Parsers.erase(parser->ParserID);
    }
}
