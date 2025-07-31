@echo off
if exist ProjectSetup/MasterSetup2.py (
    echo "ProjectSetup/MasterSetup2.py - found file"
) else (
    echo "ProjectSetup/MasterSetup2.py - file not found"
)
if exist ProjectSetup/MasterSetup.py (
    echo "ProjectSetup/MasterSetup.py - found file"
) else (
    echo "ProjectSetup/MasterSetup.py - file not found"
)
PAUSE