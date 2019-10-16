import os
script_path =  os.path.dirname(os.path.realpath(__file__))

def clean_path():
    path = os.environ['PATH'].split(":")
    for directory in path[:]:
        if "xtensa" in directory:
            path.remove(directory)
    return ":".join(path)

to_search = ('#Espressif toolchain + IDF/ADF Paths', 'export PATH=', 'export IDF_PATH=', 'export ADF_PATH=', 'export DSPLIB_PATH=')
directories = ('Filler', '/esp/xtensa-esp32-elf/bin', '/esp/esp-idf', '/esp/esp-adf', '/esp/esp-dsp/')

with open(os.path.join((os.path.expanduser('~')), '.bashrc'), 'r') as bashrc_file:
   bashrc = bashrc_file.readlines()

with open(os.path.join((os.path.expanduser('~')), '.bashrc_old'), 'w+') as old_bashrc_file:
    old_bashrc_file.writelines(bashrc)

for line in bashrc[:]:
    for search_term in to_search:
        if search_term in line:
            line_index = bashrc.index(line)
            if to_search.index(search_term) == 0:
                bashrc[line_index] = search_term + '\n'
            elif to_search.index(search_term) == 1:
                bashrc[line_index] = search_term + script_path + directories[to_search.index(search_term)] + ':' + clean_path() + '\n'
            else:
                bashrc[line_index] = search_term + script_path + directories[to_search.index(search_term)] + '\n'

with open(os.path.join((os.path.expanduser('~')), '.bashrc'), 'w+') as new_bashrc:
   new_bashrc.writelines(bashrc)

print("Source .bashrc or relog!")

# print(os.environ["PATH"])