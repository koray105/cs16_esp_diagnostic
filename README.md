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
├── build_internal.bat                  # Otomatik Derleme Senaryosu (MinGW32)
├── injector.cpp / injector.exe         # Uzaktan İş parçacığı (Remote Thread) Enjektörü
├── dllmain.cpp                         # DLL Yaşam Döngüsü & Worker Thread Yönetimi
├── viibe_config.ini                    # Kalıcı Kullanıcı Ayarları
├── ARCHITECTURE.md                     # Detaylı Mimari ve Teknik Dokümantasyon
├── README.md                           # Proje Ana Dokümantasyonu
└── src/
    ├── sdk/                            # GoldSrc Motor Yapıları ve Tipleri (ref_params_t vb.)
    ├── core/                           # Matematiksel Dönüşümler (W2S, Radar) & Loglama
    ├── render/                         # OpenGL 2D Çizim ve İskelet Render Katmanı
    ├── engine/                         # Bellek Güvenli Okuma & GoldSrc Pointer Resolver
    ├── hooks/                          # wglSwapBuffers & Dispatch Slot Hook Katmanı
    └── features/                       # Modüler Özellikler (ESP, Aimbot, Radar, Menu, Config)
```

---

## 🛠️ Gereksinimler & Kurulum (Prerequisites)

1. **İşletim Sistemi**: Windows (32-bit / 64-bit)
2. **Derleyici (Compiler)**: MinGW-w64 (32-bit `i686-w64-mingw32-g++` veya `g++.exe -m32` desteği ile)
3. **Kütüphaneler**: `opengl32`, `gdi32`, `user32`, `psapi` (Windows SDK standart kütüphaneleri)

---

## 🚀 Derleme (Building)

Proje dizininde yer alan `build_internal.bat` dosyasını çalıştırabilir veya terminal üzerinden manuel olarak derleyebilirsiniz:

```cmd
build_internal.bat
```

### Manuel Derleme Komutları:

**1. DLL Derlemesi:**
```cmd
g++.exe -shared -O2 -m32 -static-libgcc -static-libstdc++ dllmain.cpp src\core\math.cpp src\core\logger.cpp src\render\renderer.cpp src\engine\engine.cpp src\hooks\hooks.cpp src\features\esp.cpp src\features\radar.cpp src\features\misc.cpp src\features\aimbot.cpp src\features\config.cpp src\features\menu.cpp -o cs16_esp_internal.dll -lopengl32 -lgdi32 -luser32 -lpsapi
```

**2. Injector Derlemesi:**
```cmd
g++.exe -O2 -m32 -static-libgcc -static-libstdc++ injector.cpp -o injector.exe
```

---

## 🎮 Kullanım (Usage)

1. Counter-Strike 1.6'yı (`hl.exe`) başlatın.
2. Derlenen `injector.exe` dosyasını yönetici olarak çalıştırın.
3. `cs16_esp_internal.dll` dosyası otomatik olarak `hl.exe` sürecine aktarılacaktır.
4. Oyun içerisinde `INSERT` tuşuna basarak modüler GUI menüsünü açıp kapatabilirsiniz.
5. `END` tuşuna basarak DLL'i güvenli bir şekilde oyundan çıkarabilirsiniz (Clean Unhook).

---

## 💡 Teknik Mimari ve Invariantlar (Technical Highlights)

- **Crash-Proof Hooking**: Engine fonksiyonlarında inline detour yerine `hw.dll` dispatch tablosundaki `V_CalcRefdef` (RVA `hw.dll + 0x11FE36C`) ve `HUD_AddEntity` (RVA `hw.dll + 0x11FE370`) pointer değişimi tercih edilmiştir.
- **OpenGL Custom Font Renderer**: GoldSrc viewport kırpmalarını önlemek için `wglUseFontBitmapsA` yerine tamamen `GL_QUADS` ve 4 yönlü kontur geçişli (outline pass) dahili 8x8 font rasterizer kullanılmıştır.

---

## 📄 Lisans & Sorumluluk Reddi (Disclaimer)

Bu proje yalnızca **eğitim, araştırma ve tersine mühendislik (reverse engineering)** amaçlarıyla geliştirilmiştir. Kullanımdan doğabilecek tüm sorumluluk kullanıcıya aittir.
