import json
import os
from collections import defaultdict

class TemplateCreatorUtil:
    @staticmethod
    def key_file_types():
        return "fileTypes"
    
    @staticmethod
    def key_extension():
        return "extension"
    
    @staticmethod
    def key_lines():
        return "lines"
    
    @staticmethod
    def key_replace():
        return "fileNameReplaces"
    
    @staticmethod
    def value_replace():
        return "<replace>"
    
    @staticmethod
    def key_description():
        return "description"

    @staticmethod
    def add_to(jsonTemplateFilePath:str):
        print(f"Adding template to: {jsonTemplateFilePath}\n")

        print('Enter name for template will be used as command for file generation, e.g "node" will be used as -node')
        print("Return to confirm")
        name = input("> ")

        print("Enter a description")
        print("Return to confirm")
        description = input("> ")

        print("Enter full path of file to extract template from. E.g: c:/repos/Havtorn/File.h")
        print("Multiple files can be added")
        print("-c to contine with selected files")
        templates = defaultdict(list[str])
        while (True):
            extractionTarget = input("> ")
            
            if "-c" in extractionTarget:
                break
            
            extractionTarget = extractionTarget.replace('\\','/')
            try:
                with open(extractionTarget, "r") as file:
                    fileSingleString = file.read()
                    (_, extension) = extractionTarget.split('.')
                    templates[extension] = fileSingleString
            except Exception as e:
                print(e)

        print("File name replaces keys in template, e.g: ClassName is replaced by filename")
        print("-c to contine")
        fileNameReplaces = []
        while (True):
            replaceKey = input("> ")
            
            if "-c" in replaceKey:
                break
            
            fileNameReplaces.append(replaceKey)

        print("\nPreview:")
        print(f"Name of command: {name}")
        print(f"Description: {description}")
        for keys in templates:
            template = templates[keys]
            for replaceTarget in fileNameReplaces:
                    if replaceTarget in template:
                        template = template.replace(replaceTarget, TemplateCreatorUtil.value_replace())
            templates[keys] = template
            print(templates[keys])

        print("Proceed to add template? Y/N, Y = proceed, N = exit")
        while (True):
            proceed = input("> ")
            if "Y" in proceed or "y" in proceed:
                break
            else: 
                return
        
        fileTypes = list()
        for extensionKey in templates:
            fileTypes.append({
                TemplateCreatorUtil.key_extension():extensionKey,
                TemplateCreatorUtil.key_lines():templates[extensionKey]
                })

        templateJson = dict()
        with open(jsonTemplateFilePath, "r") as templateFile:
            templateJson = json.load(templateFile)
            templateFile.flush()

        templateJson[name] = {
            TemplateCreatorUtil.key_description():description,
            TemplateCreatorUtil.key_file_types():fileTypes
        }

        with open(jsonTemplateFilePath, "w") as templateFile:
            json.dump(templateJson, templateFile, indent = 4)
        
            
if __name__ == "__main__":
    TemplateCreatorUtil.add_to("FileTemplates.json")
