import io, os

steps_to_decode = {
    'rescued_step_2560.txt': 'step_2560_color_copy.cpp',
    'rescued_step_2613.txt': 'step_2613_angle_fix.cpp',
    'rescued_step_1210.txt': 'step_1210_kalip_ayir.cpp',
    'rescued_step_1242.txt': 'step_1242_makine_kalibi.cpp',
    'rescued_step_1391.txt': 'step_1391_isaret_tasi.cpp',
    'rescued_step_1397.txt': 'step_1397_kalip_bilgi.cpp',
    'rescued_step_518.txt': 'step_518_compact_mode.cpp',
    'rescued_step_5077_view.txt': 'step_5077_kalip_panel.cpp'
}

for src, dest in steps_to_decode.items():
    if os.path.exists(src):
        try:
            # UTF-16 ile oku
            content = io.open(src, 'r', encoding='utf-16').read()
            # Satır numaralarını ve öneklerini temizle
            # Genelde satırlar '4350:     GetDlgItemText...' şeklinde.
            lines = content.splitlines()
            cleaned_lines = []
            for line in lines:
                import re
                match = re.match(r'^\s*\d+:\s?(.*)$', line)
                if match:
                    cleaned_lines.append(match.group(1))
                else:
                    cleaned_lines.append(line)
            
            io.open(dest, 'w', encoding='utf-8').write('\n'.join(cleaned_lines))
            print(f"Decoded {src} -> {dest}")
        except Exception as e:
            print(f"Failed {src}: {e}")
    else:
        print(f"Not found {src}")
