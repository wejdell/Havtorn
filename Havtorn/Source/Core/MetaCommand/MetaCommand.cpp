// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "MetaCommand.h"
#include "GeneralUtilities.h"

namespace Havtorn
{
    SMetaCommand::SMetaCommand(const std::string_view input)
    {
        constexpr char separator = '/';

        I64 startIndex = 0;
        I64 endIndex = input.find_first_of(separator, startIndex);
        Parser = UGeneralUtils::HashString(input.substr(startIndex, endIndex - startIndex));
        
        startIndex = endIndex + 1;
        endIndex = input.find_first_of(separator, startIndex);
        Domain = UGeneralUtils::HashString(input.substr(startIndex, endIndex - startIndex));
        
        startIndex = endIndex + 1;
        endIndex = input.find_first_of(separator, startIndex);
        Command = UGeneralUtils::HashString(input.substr(startIndex, endIndex - startIndex));
        
        Name = input;

        constexpr char paramSeparator = '|';

        startIndex = endIndex + 1;
        endIndex = input.find_first_of(paramSeparator, startIndex);
        
        std::vector<std::string> fullParams;

        if (endIndex == std::string::npos)
            fullParams.emplace_back(input.substr(startIndex, input.size() - startIndex));

        while (endIndex != std::string::npos)
        {
            fullParams.emplace_back(input.substr(startIndex, endIndex - startIndex));
            startIndex = endIndex + 1;
            endIndex = input.find_first_of(paramSeparator, startIndex);
            
            if (endIndex == std::string::npos)
                fullParams.emplace_back(input.substr(startIndex, input.size() - startIndex));
        }

        for (I64 i = 0; i < STATIC_I64(fullParams.size()); i++)
        {
            const std::string& param = fullParams[i];

            const I64 equalityIndex = param.find_first_of('=');
            if (equalityIndex == -1)
                continue;

            std::string key = param.substr(0, equalityIndex);
            std::string value = param.substr(equalityIndex + 1, param.size() - equalityIndex);

            Parameters.emplace(key, value);
        }
    }
}
