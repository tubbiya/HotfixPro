# -*- coding: utf-8 -*-
import io

h_path = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\resource.h'
with io.open(h_path, 'r', encoding='cp1254') as f:
    h_lines = f.readlines()

new_lines = []
for line in h_lines:
    if '#define _APS_NEXT_RESOURCE_VALUE' in line:
        new_lines.append('#define IDC_GRP_ARA                     1600\n')
        new_lines.append('#define IDC_GRP_OLCU                    1601\n')
        new_lines.append('#define IDC_GRP_KESISEN                 1602\n')
        new_lines.append('#define IDC_GRP_EXPORT                  1603\n')
    new_lines.append(line)

with io.open(h_path, 'w', encoding='cp1254') as f:
    f.writelines(new_lines)
