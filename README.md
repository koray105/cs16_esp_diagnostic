# V.I.I.B.E - CS 1.6 Modular Diagnostic & ESP/Aimbot Framework

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20x86-lightgrey)
![Engine](https://img.shields.io/badge/engine-GoldSrc-orange)

Counter-Strike 1.6 (GoldSrc Motoru) için geliştirilmiş modüler, yüksek performanslı ve crash-proof 32-bit Internal Diagnostic Framework & Hile Yazılımı.

---

## 📌 Proje Özellikleri (Features)

- **Visuals (ESP)**:
  - 2D Bounding Boxes & Corner Boxes
  - Dinamik Sağlık Barları (Health Bars & Numeric Status)
  - Oyuncu İsmi, Mesafe ve Taşınan Silah Göstergeleri
  - Kemik / İskelet Takibi (Bone / Skeleton Markers)
  - Chams (Model Renklendirme)

- **Aimbot Subsystem**:
  - Gelişmiş Hedef Seçim Algoritması (FOV & Mesafe bazlı)
  - Recoil Control System (RCS - Tepme Kontrolü)
  - Yumuşatılmış Nişan (Smoothing) & Triggerbot desteği
  - Görüş Çizgisi (Line-of-Sight) Kontrolü

- **2D Tactical Radar**:
  - Oyuncunun görüş yönüne (Yaw) göre dinamik dönen 2D Taktiksel Radar
  - Düşman / Dost birim renk ayrımı

- **Misc & Telemetry**:
  - Otomatik Bunnyhop (Auto BHop)
  - Düşen/Kurulan C4 & Havada Atılan Bomba Takibi (Grenade & C4 Tracker)
  - Özelleştirilebilir Görüş Açısı (Dynamic FOV Zoom Matrix)
  - Gelişmiş Konsol ve Dosya Loglama Altyapısı

- **Interactive OpenGL GUI**:
  - Sürücü/Oyun kilitlenmesi yaşamayan tam entegre OpenGL 2D Render Engine
  - Çoklu sekme panelleri (Aimbot, Visuals, Radar, Misc, Themes, Config)
  - Sürükleyip bırakılabilir pencereler ve gelişmiş tema motoru
  - `viibe_config.ini` üzerinden kalıcı yapılandırma (INI Serialization)

---

## 📁 Proje Mimarisi (Directory Structure)

```
cs16_esp_diagnostic/
├── .gitignore                          # Git dışlama kuralları (binary/log dosyaları)
├── build_internal.bat                  # Otomatik Derleme Senaryosu (MinGW32)
├── injector.cpp                        # Uzaktan İş parçacığı (Remote Thread) Enjektörü
├── dllmain.cpp                         # DLL Giriş Noktası (DllMain & Framework Delegator)
├── ARCHITECTURE.md                     # Detaylı Mimari ve Teknik Dokümantasyon
├── README.md                           # Proje Ana Dokümantasyonu
└── src/
    ├── sdk/                            # GoldSrc Motor Yapıları ve Tipleri (ref_params_t vb.)
    ├── core/                           # Framework Yaşam Döngüsü, Girdi Yönetimi, Matematik & Loglama
    │   ├── framework.hpp / framework.cpp
    │   ├── input.hpp / input.cpp
    │   ├── math.hpp / math.cpp
    │   └── logger.hpp / logger.cpp
    ├── render/                         # OpenGL 2D Çizim ve Font Rasterizer Alt Modülleri
    │   ├── font.hpp / font.cpp
    │   ├── primitives.hpp / primitives.cpp
    │   └── renderer.hpp / renderer.cpp
    ├── engine/                         # Bellek Güvenli Okuma & GoldSrc Pointer Resolver Alt Modülleri
    │   ├── memory.hpp / memory.cpp
    │   ├── resolver.hpp / resolver.cpp
    │   ├── studio.hpp / studio.cpp
    │   ├── player.hpp / player.cpp
    │   ├── entity.hpp / entity.cpp
    │   └── engine.hpp / engine.cpp
    ├── hooks/                          # wglSwapBuffers & Dispatch Slot Hook Katmanı
    └── features/                       # Modüler Özellikler
        ├── esp.hpp / esp.cpp           # ESP ve İskelet Çizim Katmanı
        ├── aimbot.hpp / aimbot.cpp     # Aimbot ve RCS Katmanı
        ├── radar.hpp / radar.cpp       # 2D Dönen Radar Katmanı
        ├── misc.hpp / misc.cpp         # Bunnyhop ve Bomba Takip Katmanı
        ├── config.hpp / config.cpp     # INI Kayıt / Yükleme Katmanı
        └── menu/                       # Çok Sekmeli GUI & HUD Alt Modülleri
            ├── widgets.hpp / widgets.cpp
            ├── hud.hpp / hud.cpp
            ├── tabs.hpp / tabs.cpp
            └── menu.hpp / menu.cpp
```

---

## 🛠️ Gereksinimler & Kurulum (Prerequisites)

1. **İşletim Sistemi**: Windows (32-bit / 64-bit)
2. **Derleyici (Compiler)**: MinGW-w64 (32-bit `i686-w64-mingw32-g++` veya `g++.exe -m32` desteği ile)
3. **Kütüphaneler**: `opengl32`, `gdi32`, `user32`, `psapi` (Windows SDK standart kütüphaneleri)

---

## 🚀 Derleme (Building)

Proje dizininde yer alan `build_internal.bat` dosyasını çalıştırabilir veya terminal üzerinden manuel olarak derleyebilirsiniz (Çıktılar `build/` dizinine yerleştirilir):

```cmd
build_internal.bat
```

---

## 🎮 Kullanım (Usage)

1. Counter-Strike 1.6'yı (`hl.exe`) başlatın.
2. Derlenen `build\injector.exe` dosyasını yönetici olarak çalıştırın.
3. `build\cs16_esp_internal.dll` dosyası otomatik olarak `hl.exe` sürecine aktarılacaktır.
4. Oyun içerisinde `INSERT` tuşuna basarak modüler GUI menüsünü açıp kapatabilirsiniz.
5. `END` tuşuna basarak DLL'i güvenli bir şekilde oyundan çıkarabilirsiniz (Clean Unhook).

---

## 📄 Lisans & Sorumluluk Reddi (Disclaimer)

Bu proje yalnızca **eğitim, araştırma ve tersine mühendislik (reverse engineering)** amaçlarıyla geliştirilmiştir. Kullanımdan doğabilecek tüm sorumluluk kullanıcıya aittir.
