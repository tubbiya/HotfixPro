import io

path = "istek.md"
content = io.open(path, "r", encoding="cp1254", errors="ignore").read().strip()
new_entry = """

### Tarih: 2026-06-06 (Kurtarma Talebi)
- **Talep:** Yanlislikla silinmis olan 17 adet arayuz buton ve kontrol fonksiyonunun (OnBnClickedBtnAra2, OnBnClickedAra3Range, ResizeShapes, OnBnClickedBtnAra5, OnBnClickedBtnAra6, OnBnClickedBtnAra7, OnBnClickedBtnList, OnBnClickedChkListOption, OnBnClickedSimge, SetCompactMode, OnBnClickedChkKalip, OnBnClickedBtnBilgiEkle, OnBnClickedBtnKalipEkle, OnBnClickedBtnIsaretTasi, OnBnClickedBtnKalipAyir, OnBnClickedBtnMakineKalibi, OnBnClickedChkZtrass) kurtarilmasi.
- **Cozum:** Fonksiyonlarin orijinal kodlari yedeklerden ve yama dosyalarindan derlenip kurtarildi. SearchDlg.cpp icerisindeki sablonlar bu gercek kodlarla dolduruldu.
- **Durum:** Tamamlandi."""

io.open(path, "w", encoding="cp1254").write(content + new_entry)
print("Updated istek.md")
