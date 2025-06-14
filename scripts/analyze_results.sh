#!/bin/bash

echo "=== Performans Sonuçları Analizi ==="
echo ""

# Sonuç dosyalarının varlığını kontrol et
if [ ! -d "../results" ]; then
    echo "Results klasörü bulunamadı. Önce testleri çalıştırın:"
    echo "  ./run_performance_tests.sh"
    exit 1
fi

echo "Analiz edilen dosyalar:"
ls -la ../results/
echo ""

# Her version için sonuçları oku ve karşılaştır
echo "=== PERFORMANS KARŞILAŞTIRMASI ==="
echo ""

# Basit performans özeti
{
    echo "Performans Özeti"
    echo "================"
    echo "Rapor tarihi: $(date)"
    echo ""
    
    for version in version1 version2 version3; do
        result_file="../src/${version}/${version}_results.txt"
        if [ -f "$result_file" ]; then
            echo "--- ${version} ---"
            grep -E "(Versiyon|Süre|Saniyede deneme)" "$result_file" 2>/dev/null
            echo ""
        fi
    done
    
    echo ""
    echo "Sistem Bilgileri:"
    echo "=================="
    if [ -f "../results/system_info.txt" ]; then
        grep -A 5 "İşlemci Bilgileri:" ../results/system_info.txt 2>/dev/null
        grep -A 3 "Bellek Bilgileri:" ../results/system_info.txt 2>/dev/null
    fi
    
} > ../results/performance_summary.txt

echo "Performans özeti oluşturuldu: results/performance_summary.txt"

# Grafiksel karşılaştırma için veri hazırla
{
    echo "# Performans Verileri (CSV Format)"
    echo "Version,Time_Seconds,Attempts_Per_Second"
    
    for version in version1 version2 version3; do
        result_file="../src/${version}/${version}_results.txt"
        if [ -f "$result_file" ]; then
            time_val=$(grep "Süre:" "$result_file" | awk '{print $2}')
            aps_val=$(grep "Saniyede deneme:" "$result_file" | awk '{print $3}')
            echo "$version,$time_val,$aps_val"
        fi
    done
} > ../results/performance_data.csv

echo "CSV performans verisi oluşturuldu: results/performance_data.csv"

# Valgrind sonuçları varsa analiz et
echo ""
echo "=== BELLEK ANALİZİ ==="
valgrind_files=$(find ../results -name "*valgrind*" 2>/dev/null)
if [ -n "$valgrind_files" ]; then
    echo "Valgrind sonuçları bulundu:"
    echo "$valgrind_files"
else
    echo "Valgrind sonucu bulunamadı."
    echo "Bellek analizi için şu komutu çalıştırın:"
    echo "  valgrind --tool=memcheck --leak-check=full ./password_cracker_v1"
fi

# Profiling sonuçları varsa analiz et
echo ""
echo "=== PROFİLİNG ANALİZİ ==="
profiling_files=$(find ../results -name "*profiling*" 2>/dev/null)
if [ -n "$profiling_files" ]; then
    echo "Profiling sonuçları bulundu:"
    for file in $profiling_files; do
        echo ""
        echo "--- $(basename $file) ---"
        head -20 "$file" 2>/dev/null
    done
else
    echo "Profiling sonucu bulunamadı."
fi

# Öneriler
echo ""
echo "=== ÖNERİLER ==="
echo ""
echo "1. Performans Optimizasyonu:"
echo "   - Version'lar arasındaki hız farkını kontrol edin"
echo "   - CPU kullanımını mpstat sonuçlarından inceleyin"
echo ""
echo "2. Sistem İzleme:"
echo "   - results/mpstat_output.txt dosyasını inceleyin"
echo "   - CPU çekirdeklerinin etkili kullanımını kontrol edin"
echo ""
echo "3. Raporlama:"
echo "   - results/performance_summary.txt özet raporunu kullanın"
echo "   - Grafik için CSV verilerini kullanabilirsiniz"
echo ""

# İstatistik özeti
echo "=== İSTATİSTİK ÖZETİ ==="
echo ""
if [ -f "../results/performance_data.csv" ]; then
    echo "Performans karşılaştırması:"
    cat ../results/performance_data.csv | column -t -s ','
fi

echo ""
echo "Analiz tamamlandı. Tüm sonuçlar results/ klasöründe mevcuttur." 