import win32com.client
import os

try:
    print("Connecting to CorelDRAW...")
    corel = win32com.client.Dispatch("CorelDRAW.Application")
    doc = corel.ActiveDocument
    if not doc:
        print("No active document found.")
        exit(0)
    
    sel = corel.ActiveSelectionRange
    if sel.Count == 0:
        print("Please select something in CorelDRAW first.")
        exit(0)
        
    out_dir = r"C:\Users\tubbi\Dropbox\Claude\HotfixPro\scratch"
    plt_file = os.path.join(out_dir, "test_export.plt")
    
    print(f"Exporting to {plt_file}...")
    
    # Check enums
    print(f"cdrDXF = {win32com.client.constants.cdrDXF if hasattr(win32com.client.constants, 'cdrDXF') else 'Unknown'}")
    print(f"cdrHPGL = {win32com.client.constants.cdrHPGL if hasattr(win32com.client.constants, 'cdrHPGL') else 'Unknown'}")
    
    # Hardcoded values if enums fail
    cdrHPGL = 1303
    cdrSelection = 1
    
    try:
        opt = corel.CreateStructExportOptions()
        # ExportEx
        flt = doc.ExportEx(plt_file, cdrHPGL, cdrSelection, opt)
        flt.Finish()
        print("ExportEx finished successfully!")
        
        if os.path.exists(plt_file):
            print(f"File created: {plt_file} (Size: {os.path.getsize(plt_file)} bytes)")
        else:
            print("ExportEx returned without error, but FILE WAS NOT CREATED.")
            
    except Exception as e:
        print(f"ExportEx failed: {e}")
        
except Exception as e:
    print(f"General error: {e}")
