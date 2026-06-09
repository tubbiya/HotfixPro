# CorelDRAW Basit CPG Eklentisi

Bu proje, CorelDRAW için basit bir CPG eklentisi örneğidir.

## Özellikler

- MFC tabanlı dialog arayüzü
- Visual Studio Resource Editor ile düzenlenebilir UI
- Basit metin girişi ve işleme
- CorelDRAW CPG API kullanımı

## Kurulum

1. Visual Studio 2022 (veya 2019) ile projeyi açın
2. MFC kütüphanelerinin yüklü olduğundan emin olun
3. Projeyi derleyin (Build > Build Solution)
4. Oluşan .cpg dosyasını CorelDRAW'ın Plugins klasörüne kopyalayın

## Geliştirme

### UI Düzenleme

1. Solution Explorer'da `SimplePlugin.rc` dosyasına çift tıklayın
2. Resource View'da `Dialog > IDD_PLUGIN_DIALOG` öğesine çift tıklayın
3. Visual Studio'nun sürükle-bırak arayüzü ile kontrolleri düzenleyin

### Eklenti Fonksiyonları

- `CPGPlugInInfo`: Eklenti adını döndürür
- `CPGPlugInVersion`: Eklenti versiyonunu döndürür
- `CPGPlugInCommand`: Eklenti çalıştırıldığında dialog'u gösterir
- `CPGPlugInHelp`: Yardım mesajını gösterir

## Dosya Yapısı

```
CorelDraw_Plugin/
├── SimplePlugin.vcxproj      - Visual Studio proje dosyası
├── SimplePlugin.vcxproj.filters - Proje filtreleri
├── SimplePlugin.rc           - Resource dosyası (UI tanımları)
├── Resource.h                - Resource ID tanımları
├── pch.h/cpp                 - Precompiled header
├── framework.h               - Framework includes
├── PluginDialog.h/cpp        - Dialog sınıfı
├── SimplePlugin.cpp          - Ana eklenti kodu
├── dllmain.cpp              - DLL giriş noktası
└── SimplePlugin.def         - Export tanımları
```

## Notlar

- Proje MFC Dynamic Library kullanır
- CorelDRAW'ın CPG API'sini kullanır
- .cpg uzantılı DLL olarak derlenir

## Visual Studio'da Açma

1. `SimplePlugin.vcxproj` dosyasına çift tıklayın
2. Proje Visual Studio'da açılacaktır
3. Build > Build Solution ile derleyin

## UI Düzenleme (Sürükle-Bırak)

1. Solution Explorer → `SimplePlugin.rc` → çift tıkla
2. Resource View → Dialog → `IDD_PLUGIN_DIALOG` → çift tıkla
3. Toolbox'tan kontrol ekleyin (Button, TextBox, Label vb.)
4. Properties penceresinden özellikleri düzenleyin
