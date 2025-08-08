import os
import subprocess
import platform
import pprint
import sys

print(f"python installation: {sys.executable}")  # Shows which Python installation is running
print("")
print(f"module search: {sys.path}")       # Shows where Python looks for modules
print("")

from ProjectSetup.SetupPython import PythonConfiguration as PythonRequirements
PythonRequirements.Validate()