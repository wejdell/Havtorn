import os
import subprocess
import platform
import pprint
import sys
print(os.path.abspath("./"))
#sys.path.append(os.path.abspath("./"))
#sys.path.append(os.path.abspath("./SetupRequirements/Python/Scripts/"))
print(f"python installation: {sys.executable}")  # Shows which Python installation is running
print("")
print(f"module search: {sys.path}")       # Shows where Python looks for modules
print("")

from SetupPython import PythonConfiguration as PythonRequirements
PythonRequirements.Validate()