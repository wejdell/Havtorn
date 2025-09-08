@echo off
::TODO: figure out how to handle the update scenario. To update: vs_BuildTools.exe update --passive --wait --add Microsoft.VisualStudio.Workload.VCTools;includeRecommended
vs_BuildTools.exe --passive --wait --add Microsoft.VisualStudio.Workload.VCTools;includeRecommended
