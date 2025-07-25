import os
import sys
import subprocess
from pathlib import Path
import pprint

print(f"testvar is {os.environ["testvar"]}, from TestPython.py")
print("starting subprocess TestSubProcessScript.bat from TestPython.py")
subprocess.call([os.path.abspath("./TestSubProcessScript.bat"), "nopause"])