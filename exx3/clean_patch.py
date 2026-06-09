import io
content = io.open('decompiled_patch.py', 'r', encoding='utf-8').read().strip()
while True:
    if content.startswith('\ufeff'):
        content = content[1:]
    elif content.startswith('"') and content.endswith('"'):
        content = content[1:-1]
    elif content.startswith("'") and content.endswith("'"):
        content = content[1:-1]
    else:
        break
io.open('decompiled_patch.py', 'w', encoding='utf-8').write(content)
print("Done cleaning")
