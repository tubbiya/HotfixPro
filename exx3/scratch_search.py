# -*- coding: utf-8 -*-
import codecs

with codecs.open('istek.md', 'a', 'utf-8') as f:
    f.write('\n\n### Ek Talep 56\n\n- Bilgi yazısının konumlandırılmasında en dış kalıp karesinin doğru tespit edilebilmesi için en büyük nesne referanslama mantığı getirildi.\n- Hem C++ (`SearchDlg.cpp`) hem de VBA (`ProB.frm`) tarafında seçilen nesneler arasındaki en büyük alana (genişlik * yükseklik) sahip tek nesne otomatik olarak kalıp karesi olarak belirlendi.\n- Bilgi yazısı ve ilişkili listenin sol üst referans koordinatları (`kalipLeft`, `kalipTop`), bu en büyük nesnenin sınırlarına göre atandı. Bu sayede seçim alanı içerisinde alakasız küçük nesneler, yazılar veya işaret taşları bulunsa dahi, bilgi metninin her zaman tam olarak dış kalıp karesinin sol üstünden 5 mm yukarıda hizalanması kesinleştirildi.\n')
print("Successfully appended Ek Talep 56 to istek.md")
