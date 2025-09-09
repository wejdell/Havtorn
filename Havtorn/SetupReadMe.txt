********************************************
************ HAVTORN SETUP INFO ************
********************************************

Havtorn requires a few programs for setting up and building project files.
Provided in the repository are some scripts that verify that these requirements are available and if not, prompts for install permissions.
After pulling updates it is recommended to run GenerateProject and BuildProject.

- ! IMPORTANT !
The setup process currently supports: Windows 10 and 11.
It is still a work in progress so some requirements may require manual steps or troubleshooting.

- Troubleshooting:
If a process window closes unexpectedly that is typically a sign of an error. 
A recommendation is to not close the process window once it is done before looking at the logs.
Any red or yellow text is a clear indication of error. 

- SetupProject.bat - Required step, initial step
This runs through verification of all requirements and generates the project files.
After SetupProject has completed successfully you can run BuildProject.bat
If you start from a clean repository you'll need to run this again.

- GenerateProject.bat
Generates files for a visual studio project as well as files for external libraries.
Requirement for: BuildProject.bat

- BuildProject.bat
Builds an executable of Havtorn. Found inside Havtorn/Bin/ called Launcher.exe.
NOTE: only builds the Havtorn Editor.

- False positives:
When using Build Tools without Visual Studio:
    CMake Error:
    Generator
        Visual Studio 17 2022
    could not find specified instance of Visual Studio:
        C:/Program Files/Microsoft Visual Studio/2022/Community

    If the script continues, everything is fine. This error has something to do with the CMake files used

