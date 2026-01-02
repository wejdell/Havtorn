import json
import os.path
import time
from datetime import datetime
from collections import defaultdict
from ValidationUtils import ValidationUtil
from FileCreatorResources import FileCreatorResources

# TODO add support for updating a template with a "fileTypes"
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
    def get_templates_map_from(jsonPath:str):
        templateJson = dict()
        with open(jsonPath, "r") as templateFile:
            templateJson = json.load(templateFile)
        return templateJson
        
    @staticmethod
    def get_default_file_templates_path():
        return "FileTemplates.json"

    @staticmethod
    def print_name_replace_example():
        print(f"Example - Class Name: <ExampleClass> will be replaced with {TemplateCreatorUtil.value_replace()}:")
        print(f"  class ExampleClass --> class {TemplateCreatorUtil.value_replace()}")
        print("  {")
        print(f"    ~ExampleClass(); --> ~{TemplateCreatorUtil.value_replace()}();")
        print("    DoSomething();")
        print("  }")

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
            name = input(FileCreatorResources.INPUT_CHARACTERS)
            if name == "":
                FileCreatorResources.print_error("must give a name")
                continue
            break

        print()
        print('Provide a short description, e.g: "Class-template for inheriting from ..."')
        description = input(FileCreatorResources.INPUT_CHARACTERS)

        print()
        print('Enter full path of file to extract template from, e.g: "c:/repos/Havtorn/File.h"')
        print("Accepts path with '/' and '\\'")
        print("1 file can be added per extension-type. Empty input/Return to continue")
        templates = defaultdict(str)
        while (True):
            extractionTarget = input(FileCreatorResources.INPUT_CHARACTERS)
            if extractionTarget == "":
                break
            (extension, contents) = TemplateCreatorUtil.extract_extension_and_contents(extractionTarget)
            if extension == "" or contents == "":
                continue
            templates[extension] = contents
            print("File added")

        print()
        print("Enter class name, struct name, etc")
        TemplateCreatorUtil.print_name_replace_example()
        print("Return to continue")
        fileNameReplaces = ""
        while (True):
            replaceKey = input(FileCreatorResources.INPUT_CHARACTERS)
            if replaceKey == "":
                FileCreatorResources.print_error("must enter name to replace, such as class or struct name")
                continue
            fileNameReplaces = replaceKey  
            break
        templates = TemplateCreatorUtil.update_templates_with_replace_value(fileNameReplaces, TemplateCreatorUtil.value_replace(), templates)

        print()
        print("Generating preview...")
        time.sleep(0.77)
        TemplateCreatorUtil.display_preview(name, description, templates)

        print("\nProceed to add template to file? Return to proceed or 'n' to exit")
        while (True):
            proceed = input(FileCreatorResources.INPUT_CHARACTERS)
            if proceed == "":
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
    def update_templates_with_replace_value(replaceTarget:str, replaceValue:str, templates:dict[str,str]):
        updatedTemplates = templates
        for key in updatedTemplates:
            template = updatedTemplates[key]
            if replaceTarget in template:
                    template = template.replace(replaceTarget, replaceValue)
            updatedTemplates[key] = template
        return updatedTemplates
    
    @staticmethod
    def extract_extension_and_contents(extractionTargetPath:str):
        extractionTargetPath = extractionTargetPath.replace('\\','/')
        try:
            (_, extension) = extractionTargetPath.split('.')
        except Exception as e:
            FileCreatorResources.print_error("Incompatible file")
            return ("", "")
        
        # Limitation: License should use current year
        # TODO: handle year as a number 20XX
        havtornLicense = FileCreatorResources.get_havtorn_license()
        currentYear = datetime.now().year
        yearSymbol = "[[year]]"
        havtornLicense = havtornLicense.replace(str(currentYear), yearSymbol)

        try:
            with open(extractionTargetPath, "r") as file:
                fileSingleString = file.read()
                for y in range(2020, currentYear + 1):
                    licenseAdjustedForYear = havtornLicense.replace(yearSymbol, str(y))
                    fileSingleString = fileSingleString.replace(licenseAdjustedForYear, "")

        except Exception as e:
            if e is FileNotFoundError:
                FileCreatorResources.print_error("File does not exist")
            else:
                print(e)
            return ("", "")
        
        return (f'.{extension}', fileSingleString)
        
    @staticmethod
    def display_preview(name:str, description:str, templates:dict[str,str]):
        print(f"----- Preview -----")
        print(f"{name} - {description}")
        for key in templates:
            print(f"--- {key} ---")
            print(f"{templates[key]}", end = "")
        return
    
    @staticmethod
    def print_templates(jsonTemplateFilePath:str):
        templateJson = dict()
        with open(jsonTemplateFilePath, "r") as templateFile:
           templateJson = json.load(templateFile)

        print("Current Templates:")
        for template in templateJson:
            fileTypes = ""
            for types in templateJson[template][TemplateCreatorUtil.key_file_types()]:
                fileTypes += f"{types[TemplateCreatorUtil.key_extension()]},"
            print(f"  name: {template}, desc: {templateJson[template][TemplateCreatorUtil.key_description()]}, file types: [{fileTypes}]")
        
            
if __name__ == "__main__":
    TemplateCreatorUtil.print_templates(TemplateCreatorUtil.get_default_file_templates_path())
    TemplateCreatorUtil.create_and_add_from_input(TemplateCreatorUtil.get_default_file_templates_path())
    print("Closing ...")
    time.sleep(0.5)
