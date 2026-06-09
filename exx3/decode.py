import io
content = io.open('decompiled_patch.py', 'r', encoding='utf-8').read().strip()
if content.startswith('"') and content.endswith('"'):
    content = content[1:-1]
elif content.startswith("'''") and content.endswith("'''"):
    content = content[3:-3]
elif content.startswith('"""') and content.endswith('"""'):
    content = content[3:-3]
io.open('decompiled_patch.py', 'w', encoding='utf-8').write(content)
