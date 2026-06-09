# -*- coding: utf-8 -*-
import io
import re

h_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\resource.h'
with io.open(h_path, 'r', encoding='cp1254') as f:
    lines = f.readlines()

used_ids = set()
duplicates = []

# Find used IDs
for i, line in enumerate(lines):
    match = re.search(r'#define\s+(\w+)\s+(\d+)', line)
    if match:
        name = match.group(1)
        val = int(match.group(2))
        
        # Don't touch Next values at the bottom
        if name.startswith('_APS_'):
            continue
            
        if val in used_ids:
            duplicates.append(i)
        else:
            used_ids.add(val)

# Fix duplicates
next_id = 1500
while next_id in used_ids:
    next_id += 1

for idx in duplicates:
    match = re.search(r'#define\s+(\w+)\s+(\d+)', lines[idx])
    name = match.group(1)
    
    # ensure next_id is not used
    while next_id in used_ids:
        next_id += 1
        
    lines[idx] = '#define {} {}\n'.format(name, next_id)
    used_ids.add(next_id)
    print("Fixed duplicate:", name, "new id:", next_id)

with io.open(h_path, 'w', encoding='cp1254') as f:
    f.writelines(lines)
