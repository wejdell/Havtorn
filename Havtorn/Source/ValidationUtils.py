import re

class ValidationUtil:
    @staticmethod
    def validate_file_name(filename):
        # src: https://labex.io/tutorials/python-how-to-validate-file-and-folder-names-419733
        checks = [
            ## Length check
            len(filename) <= 255,
            ## No reserved names
            filename.lower() not in [
                'con', 
                'prn', 
                'aux', 
                'nul', 
                'com1',
                'com2',
                'com3',
                'com4',
                'com5',
                'com6',
                'com7',
                'com8',
                'com9',
                'com0',
                'lpt1',
                'lpt2',
                'lpt3',
                'lpt4',
                'lpt5',
                'lpt6',
                'lpt7',
                'lpt8',
                'lpt9',
                'lpt0',
                ],
            ## No special characters
            re.match(r'^[a-zA-Z0-9_\-\.]+$', filename) is not None,
            ## No hidden files or directories
            not filename.startswith('.'),
            not filename.endswith('.'),
        ]

        return all(checks)

    @staticmethod
    def validate_directory_name(direcotryname):
        return validate_file_name(direcotryname)

    @staticmethod
    def validate_file_extension(extension):
        checks = [
            ## Permitted
            extension.lower() in [
                'cpp', 
                'c', 
                'h', 
                'hpp', 
                'hlsl',
                'hlsli',
                'txt',
                'py',
                'cs',
                ],
            ## No special characters
            re.match(r'^[a-zA-Z0-9]+$', extension) is not None,
            not extension.endswith('.'),
        ]

        return all(checks)


if __name__ == "__main__":

    while(True):
        print("Test filename validation. pick one:")
        print("\t1 filename/directory")
        print("\t2 extension")
        toTest=input()
        match toTest:
            case "1":
                print("Enter file/directory name:")
                name=input()
                print(f">> {ValidationUtil.validate_file_name(name)}")
                print()
            case "2":
                print("Enter extension name, exclude '.', e.g: .txt -> txt")
                ext=input()
                print(f">> {ValidationUtil.validate_file_extension(ext)}")
                print()