# C++ Paralel Brute-Force Şifre Kırma Simülasyonu

## Proje Yapısı
```
projekt/
├── src/
│   ├── version1/          # Tek iş parçacıklı versiyon
│   │   ├── main.cpp       # Ana program
│   │   ├── Makefile       # Derleme dosyası
│   │   └── version1_results.txt  # Sonuçlar
│   ├── version2/          # Çoklu iş parçacıklı versiyon  
│   │   ├── main.cpp
│   │   ├── Makefile
│   │   └── version2_results.txt
│   ├── version3/          # Vektörleştirilmiş versiyon
│   │   ├── main.cpp
│   │   ├── Makefile
│   │   └── version3_results.txt
│   └── common/            # Ortak fonksiyonlar
│       ├── password_cracker.h
│       └── password_cracker.cpp
├── scripts/               # Bash betikleri
├── results/               # Özet raporlar
└── README.md
```

## Nasıl Çalıştırılır

### Adım 1: Tüm versiyonları derle
```bash
cd scripts
chmod +x build_all.sh
./build_all.sh
```

### Adım 2: Performans testlerini çalıştır
```bash
./run_performance_tests.sh
```

### Adım 3: Sistem izleme komutlarını çalıştır
```bash
./monitor_system.sh
```

## Versiyon Açıklamaları

- **Version 1**: Temel tek iş parçacıklı brute-force
- **Version 2**: Çoklu iş parçacığı ile paralel hesaplama
- **Version 3**: SIMD vektörleştirme ile optimizasyon

Her versiyon aynı problemi çözer: Verilen hash'e karşılık gelen şifreyi bulur.

## Performans Karşılaştırması

Test şifresi: `abc1` (4 karakter)

| Versiyon | Süre (saniye) | Saniyede Deneme | Hızlanma |
|----------|---------------|-----------------|----------|
| Version 1 (Single Thread) | 0.046 | 15.1M | 1x |
| Version 2 (Multi Thread) | 0.001 | 68.3M | 4.5x |
| Version 3 (Vectorized) | 0.001 | 93.6M | 6.2x |

Çoklu iş parçacığı kullanımı 4.5x, vektörleştirme ile birlikte 6.2x hızlanma sağlandı. 