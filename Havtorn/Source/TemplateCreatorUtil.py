import json
import os.path
import time
from collections import defaultdict
from ValidationUtils import ValidationUtil

class TemplateCreatorUtil:
    # Json object example:
    #  "command-name":{ # object key is the name used for the template and command
    #       "description":"string",
    #       "fileTypes":[
    #                    {
    #                        "extension":"string: file type associated extension",
    #                        "content":"string: contents of template"
    #                    }
    #                   ]
    #               }
    @staticmethod
    def key_file_types():
        return "fileTypes"
    
    @staticmethod
    def key_extension():
        return "extension"
    
    @staticmethod
    def key_template_content():
        return "content"
    
    @staticmethod
    def value_replace():
        return '{{fileName}}'
    
    @staticmethod
    def key_description():
        return "description"

    @staticmethod
    def create_and_add_from_input(jsonTemplateFilePath:str):
        if not os.path.isfile(jsonTemplateFilePath):
            print(f"! Error with {jsonTemplateFilePath}")
            return
        
        print()
        print(f"Adding template to file: {jsonTemplateFilePath}")
        print()
        print('Enter name for template. Recommended to use single word, e.g: "node"')
        while (True):
            name = input("> ")
            if name == "":
                print("! must give a name")
                continue
            break

        print()
        print('Provide a short description, e.g: "Class-template for inheriting from Node"')
        description = input("> ")

        print()
        print('Enter full path of file to extract template from, e.g: "c:/repos/Havtorn/File.h"')
        print("\\ is automatically replaced by /")
        print("Multiple files can be added. Return to continue")
        templates = defaultdict(str)
        while (True):
            extractionTarget = input("> ")
            if extractionTarget == "":
                break
            (extension, contents) = TemplateCreatorUtil.extract_extension_and_contents(extractionTarget)
            if extension == "" or contents == "":
                continue
            templates[extension] = contents

        print()
        print("Enter words that will be replaced with file name when template is used")
        print("Multiple can be added. Return to continue")
        fileNameReplaces = []
        while (True):
            replaceKey = input("> ")
            if replaceKey == "":
                break
            fileNameReplaces.append(replaceKey)  
        templates = TemplateCreatorUtil.update_templates_with_replace_value(fileNameReplaces, TemplateCreatorUtil.value_replace(), templates)

        print()
        print("Generating preview...")
        time.sleep(0.77)
        TemplateCreatorUtil.display_preview(name, description, templates)

        print("\nProceed to add template to file? y = proceed, anything else = exit")
        while (True):
            proceed = input("> ")
            if "Y" in proceed.capitalize():
                break
            else: 
                return
        
        TemplateCreatorUtil.to_existing_json(jsonTemplateFilePath, name, description, templates)
        return
    
    @staticmethod
    def to_existing_json(jsonTemplateFilePath:str, name:str, description:str, templates:dict[str,str]):
        templateJson = dict()
        with open(jsonTemplateFilePath, "r") as templateFile:
            templateJson = json.load(templateFile)
            templateFile.flush()

        templateJson[name] = {
            TemplateCreatorUtil.key_description():description,
            TemplateCreatorUtil.key_file_types():TemplateCreatorUtil.convert_templates_to_json(templates)
        }

        with open(jsonTemplateFilePath, "w") as templateFile:
            json.dump(templateJson, templateFile, indent = 4)
        return

    @staticmethod
    def convert_templates_to_json(templates:dict):
        fileTypes = list()
        for extensionKey in templates:
            fileTypes.append({
                TemplateCreatorUtil.key_extension():extensionKey,
                TemplateCreatorUtil.key_template_content():templates[extensionKey]
                })
        return fileTypes

    @staticmethod
    def update_templates_with_replace_value(replaceTargets:list[str], replaceValue:str, templates:dict[str,str]):
        updatedTemplates = templates
        for keys in updatedTemplates:
            template = updatedTemplates[keys]
            for replaceTarget in replaceTargets:
                    if replaceTarget in template:
                        template = template.replace(replaceTarget, replaceValue)
            updatedTemplates[keys] = template
        return updatedTemplates
    
    @staticmethod
    def extract_extension_and_contents(extractionTargetPath:str):
        extractionTargetPath = extractionTargetPath.replace('\\','/')
        try:
            (_, extension) = extractionTargetPath.split('.')
        except Exception as e:
            print(f"! Incompatible file")
            return ("", "")
        
        try:
            with open(extractionTargetPath, "r") as file:
                fileSingleString = file.read()
        except Exception as e:
            if e is FileNotFoundError:
                print(f"! File does not exist")
            else:
                print(e)
            return ("", "")
        
        return (extension, fileSingleString)
        
    @staticmethod
    def display_preview(name:str, description:str, templates:dict[str,str]):
        print(f"----- Preview -----")
        print(f"{name} - {description}")
        for key in templates:
            print(f"--- .{key} ---")
            print(f"{templates[key]}", end = "")
        return
            
if __name__ == "__main__":
    TemplateCreatorUtil.create_and_add_from_input("FileTemplates.json")
