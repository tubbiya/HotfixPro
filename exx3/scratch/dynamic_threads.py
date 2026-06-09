import io
import re

path_cpp = r'c:\Users\tubbi\Dropbox\Claude\HotfixPro\SearchDlg.cpp'
with io.open(path_cpp, 'r', encoding='utf-8', errors='ignore') as f:
    text_cpp = f.read()

old_thread_logic = """            // Multithreading ile hizlandirma (8 is parcacigi)
            int numThreads = 8;"""

new_thread_logic = """            // Multithreading ile hizlandirma (Dinamik is parcacigi)
            int hwThreads = std::thread::hardware_concurrency();
            if (hwThreads == 0) hwThreads = 8;
            
            // Cok az obje varsa gereksiz thread acmayalim
            int numThreads = hwThreads;
            if (count < 1000) {
                numThreads = 2;
            }
            if (count < 100) {
                numThreads = 1;
            }"""

text_cpp = text_cpp.replace(old_thread_logic, new_thread_logic)

with io.open(path_cpp, 'w', encoding='utf-8') as f:
    f.write(text_cpp)

print("Dynamic thread allocation added.")
