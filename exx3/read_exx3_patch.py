import io
content = io.open('exx3/scratch/patch_resize_export.py', 'r', encoding='utf-8').read()
content = content.replace("with io.open(path_cpp, 'r', encoding='utf-8') as f:\n    text_cpp = f.read()", "text_cpp = ''")
content = content.replace("with io.open(path_cpp, 'r', encoding='utf-8') as f:", "if True:")
content = content.replace("with io.open(path_cpp, 'w', encoding='utf-8') as f:", "if False:")
content = content.replace("f.write(text_cpp)", "pass")
local_vars = {}
exec(content, globals(), local_vars)

if 'new_resize_shapes' in local_vars:
    io.open('new_resize_shapes.cpp', 'w', encoding='utf-8').write(local_vars['new_resize_shapes'])
    print("Saved new_resize_shapes.cpp")
if 'new_export' in local_vars:
    io.open('new_export.cpp', 'w', encoding='utf-8').write(local_vars['new_export'])
    print("Saved new_export.cpp")
