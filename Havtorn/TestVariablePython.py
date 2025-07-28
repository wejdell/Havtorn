import os
import sys
import subprocess
from pathlib import Path
import pprint

print("starting subprocess TestVariableSubScript.bat from TestVariablePython.py")
subprocess.call([os.path.abspath("./TestVariableSubScript.bat"), "nopause"])
print(f"testVariable is {os.environ["testVariable"]}, set in subprocess read from from TestVariablePython.py")
input()