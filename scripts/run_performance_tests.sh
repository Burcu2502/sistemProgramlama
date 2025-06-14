#!/bin/bash

echo "=== C++ Paralel Brute-Force Performans Testleri ==="
echo ""

# Results klasörünü temizle
rm -f ../results/*.txt

echo "Sistem bilgileri toplanıyor..."
echo "İşlemci: $(cat /proc/cpuinfo | grep 'model name' | head -1 | cut -d ':' -f2 | xargs)"
echo "CPU çekirdek sayısı: $(nproc)"
echo "Bellek: $(free -h | grep Mem | awk '{print $2}')"
echo ""

# Her versiyonu sırayla çalıştır
versions=("version1" "version2" "version3")
names=("Tek İş Parçacıklı" "Çoklu İş Parçacıklı" "Vektörleştirilmiş")

for i in "${!versions[@]}"; do
    version=${versions[$i]}
    name=${names[$i]}
    
    echo "=== $name Versiyon Çalıştırılıyor ==="
    echo "Başlangıç zamanı: $(date)"
    
    cd ../src/$version
    
    # Gprof profiling için derleme (opsiyonel)
    echo "Profiling için yeniden derleniyor..."
    make clean > /dev/null 2>&1
    make CXXFLAGS="-std=c++17 -O2 -Wall -Wextra -pg" > /dev/null 2>&1
    
    # Programı çalıştır ve zamanı ölç
    echo "Program çalıştırılıyor..."
    time_output=$(time (./${version/version/password_cracker_v} 2>&1) 2>&1)
    
    echo "Bitiş zamanı: $(date)"
    echo "Zaman bilgisi:"
    echo "$time_output" | grep real
    echo ""
    
    # Profiling sonuçları (varsa)
    if [ -f gmon.out ]; then
        echo "Profiling sonuçları kaydediliyor..."
        gprof ./${version/version/password_cracker_v} gmon.out > ../../results/${version}_profiling.txt
        rm gmon.out
    fi
    
    cd ../../scripts
    
    # Kısa bekleme
    sleep 2
done

echo "=== TÜM TESTLER TAMAMLANDI ==="
echo ""
echo "Sonuçlar ../results/ klasöründe saklandı:"
ls -la ../results/

echo ""
echo "Detaylı sistem izleme için:"
echo "  ./monitor_system.sh" 