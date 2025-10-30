import json
import os
from collections import defaultdict

from ValidationUtils import ValidationUtil

class TemplateJsonUtil:
    @staticmethod
    def key_file_types_object():
        return "fileTypes"
    
    @staticmethod
    def key_file_types_object_extension():
        return "extension"
    
    @staticmethod
    def key_file_types_object_lines():
        return "lines"
    
    @staticmethod
    def key_file_types_object_file_name_replaces():
        return "fileNameReplaces"
    
    @staticmethod
    def value_file_types_object_file_name_replaces():
        return "fileName"
    
    @staticmethod
    def key_file_types_object_description():
        return "description"
    

class TemplateCreatorUtil:
    # set name/command, e.g: "node" will be used as -node for file generator
    # enter a description, no return. 
    # [...] enter full path to template base, -c to continue
    # Enter keys to replace with file name, e.g: StructName
    # Show preview example
    #   command: name
    #   description: 
    #   h : blabla with fileNameReplaces
    #   cpp: blabla
    #   
    # y/n to accept: y = add to templates, n = reset
    # y: Add to FileTemplates.json
    # n: Reset
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
        print(f"Name & command: {name}")
        print(f"Description: {description}")
        for keys in templates:
            template = templates[keys]
            for replaceTarget in fileNameReplaces:
                    if replaceTarget in template:
                        template = template.replace(replaceTarget, TemplateJsonUtil.value_file_types_object_file_name_replaces())
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
            fileTypes += {
                TemplateJsonUtil.key_file_types_object_extension:extensionKey,
                TemplateJsonUtil.key_file_types_object_lines:templates[extensionKey]
            }
        
        templateMap = dict()
        templateMap[name] = {
            TemplateJsonUtil.key_file_types_object_description():description,
            TemplateJsonUtil.key_file_types_object():fileTypes
        }
        
        with open(jsonTemplateFilePath, "r+") as templateFile:
            templateJson = json.load(templateFile)
            templateJson[name] = templateMap
            json.dump(templateJson, templateFile)
        #try:
 
       #except Exception as e:
        #    print(e)
        
            
if __name__ == "__main__":
    TemplateCreatorUtil.add_to("FileTemplates.json")
