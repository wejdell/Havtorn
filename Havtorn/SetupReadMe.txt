***************************************
************ HAVTORN SETUP ************ 
***************************************

Havtorn requires a few programs for setting up and building project files.
Provided are some scripts that verify that these requirements are available and if not prompts for install permissions.

! IMPORTANT !
The setup process currently supports Windows 10 and 11.
It is still a work in process so some requirements may require manual steps or troubleshooting.  

SetupProject.bat - Required step
This is a 1 time step and runs through verification of all requirements and generates the project files.
After this has completed successfully you can run BuildProject.bat
If you start from a clean repository you'll need to run this again.

GenerateProject.bat
Generates files for a visual studio project as well as files for external libraries.
Requirement for: BuildProject.bat

BuildProject.bat
Builds an executable of Havtorn. Found inside Havtorn/Bin/ called Launcher.exe.
NOTE: currently only setup to build the Havtorn Editor executable.