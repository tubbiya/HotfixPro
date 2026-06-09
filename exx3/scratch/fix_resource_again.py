# -*- coding: utf-8 -*-
import io

h_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\resource.h'
with io.open(h_path, 'r', encoding='cp1254') as f:
    lines = f.readlines()

new_lines = []
for line in lines:
    # Skip my bad insertions
    if '1600' in line or '1601' in line or '1602' in line or '1603' in line:
        continue
    new_lines.append(line)
    if '#define IDC_GRP_ARA' in line:
        new_lines.append('#define IDC_GRP_OLCU                    1601\n')
        new_lines.append('#define IDC_GRP_KESISEN                 1602\n')
        new_lines.append('#define IDC_GRP_EXPORT                  1603\n')

with io.open(h_path, 'w', encoding='cp1254') as f:
    f.writelines(new_lines)
