# TODO: add to CMakeList.txt
substring="set(ENGINE_FILES\n"
addLine="\t${ENGINE_FOLDER}Application/NewProcess.cpp\n"
fileAsLineList=list[str]
with open("TestFile.cpp", "r") as file: 
    fileAsLineList = file.readlines()
    print(fileAsLineList)
    fileAsLineList.insert(fileAsLineList.index(substring) + 1, addLine)
    # fileAsLineList.insert(len(fileAsLineList)+1, addLine) safe if this happens
    file.flush()
with open("TestFile.cpp", "w") as file:
    file.writelines(fileAsLineList)