#!/bin/bash

echo "=== Sistem İzleme ve Analiz Araçları ==="
echo ""

# Sistem bilgilerini kaydet
echo "Sistem bilgileri kaydediliyor..."
{
    echo "=== SİSTEM BİLGİLERİ ==="
    date
    echo ""
    echo "İşlemci Bilgileri:"
    cat /proc/cpuinfo | grep -E "(model name|processor|cpu MHz|cache size)" | head -20
    echo ""
    echo "Bellek Bilgileri:"
    free -h
    echo ""
    echo "Disk Bilgileri:"
    df -h
    echo ""
} > ../results/system_info.txt

echo "Sistem bilgileri kaydedildi: results/system_info.txt"

echo ""
echo "=== CPU İzleme Araçları ==="
echo ""

# mpstat komutu
if command -v mpstat &> /dev/null; then
    echo "mpstat ile CPU kullanımı analiz ediliyor..."
    echo "5 saniye boyunca 1 saniye aralıklarla CPU kullanımı:"
    mpstat 1 5 | tee ../results/mpstat_output.txt
    echo ""
else
    echo "mpstat komutu bulunamadı. Yüklemek için: sudo apt install sysstat"
fi

# vmstat komutu
if command -v vmstat &> /dev/null; then
    echo "vmstat ile sistem performansı analiz ediliyor..."
    echo "5 saniye boyunca 1 saniye aralıklarla sistem durumu:"
    vmstat 1 5 | tee ../results/vmstat_output.txt
    echo ""
else
    echo "vmstat komutu bulunamadı."
fi

# iostat komutu
if command -v iostat &> /dev/null; then
    echo "iostat ile disk I/O analiz ediliyor..."
    echo "Disk I/O istatistikleri:"
    iostat -x 1 3 | tee ../results/iostat_output.txt
    echo ""
else
    echo "iostat komutu bulunamadı."
fi

echo "=== GERÇEK ZAMANLI İZLEME ==="
echo ""
echo "Aşağıdaki komutları ayrı terminallerde çalıştırabilirsiniz:"
echo ""
echo "1. CPU kullanımını izlemek için:"
echo "   watch -n 1 'cat /proc/loadavg; echo; mpstat | tail -1'"
echo ""
echo "2. Bellek kullanımını izlemek için:"
echo "   watch -n 1 'free -h; echo; ps aux --sort=-%mem | head -10'"
echo ""
echo "3. top komutu ile detaylı sistem izleme:"
echo "   top -d 1"
echo ""
echo "4. Program çalışırken CPU kullanımını izlemek için:"
echo "   watch -n 1 'ps aux | grep password_cracker'"
echo ""

# Interaktif menü
echo "İnteraktif izleme menüsü (Ctrl+C ile çıkış):"
echo "1) top komutu"
echo "2) htop komutu (varsa)"
echo "3) Sürekli CPU izleme"
echo "4) Sürekli bellek izleme"
echo "5) Çıkış"
echo ""

read -p "Seçiminizi yapın (1-5): " choice

case $choice in
    1)
        echo "top komutu başlatılıyor..."
        top
        ;;
    2)
        if command -v htop &> /dev/null; then
            echo "htop komutu başlatılıyor..."
            htop
        else
            echo "htop komutu bulunamadı. Yüklemek için: sudo apt install htop"
        fi
        ;;
    3)
        echo "Sürekli CPU izleme başlatılıyor (Ctrl+C ile çıkış)..."
        while true; do
            clear
            echo "=== CPU Kullanımı ==="
            date
            echo ""
            cat /proc/loadavg
            echo ""
            if command -v mpstat &> /dev/null; then
                mpstat | tail -1
            fi
            echo ""
            echo "Çalışan işlemler:"
            ps aux --sort=-%cpu | head -10
            sleep 2
        done
        ;;
    4)
        echo "Sürekli bellek izleme başlatılıyor (Ctrl+C ile çıkış)..."
        while true; do
            clear
            echo "=== Bellek Kullanımı ==="
            date
            echo ""
            free -h
            echo ""
            echo "En çok bellek kullanan işlemler:"
            ps aux --sort=-%mem | head -10
            sleep 2
        done
        ;;
    5)
        echo "Çıkış yapılıyor..."
        ;;
    *)
        echo "Geçersiz seçim!"
        ;;
esac

echo ""
echo "Sistem izleme tamamlandı. Sonuçlar results/ klasöründe." 