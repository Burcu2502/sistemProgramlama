#!/bin/bash

echo "=== C++ Paralel Brute-Force Projesi Derleme Scripti ==="
echo ""

# Results klasörünü oluştur
mkdir -p ../results

# Derleme başarısını takip et
success_count=0
total_versions=3

echo "Version 1 derleniyor (Tek İş Parçacıklı)..."
cd ../src/version1
if make clean && make; then
    echo "✓ Version 1 başarıyla derlendi"
    success_count=$((success_count + 1))
else
    echo "✗ Version 1 derlemesi başarısız"
fi
echo ""

echo "Version 2 derleniyor (Çoklu İş Parçacıklı)..."
cd ../version2
if make clean && make; then
    echo "✓ Version 2 başarıyla derlendi"
    success_count=$((success_count + 1))
else
    echo "✗ Version 2 derlemesi başarısız"
fi
echo ""

echo "Version 3 derleniyor (Vektörleştirilmiş)..."
cd ../version3
if make clean && make; then
    echo "✓ Version 3 başarıyla derlendi"
    success_count=$((success_count + 1))
else
    echo "✗ Version 3 derlemesi başarısız"
fi
echo ""

# Sonuç özeti
echo "=== DERLEME SONUÇLARI ==="
echo "Başarılı: $success_count/$total_versions"

if [ $success_count -eq $total_versions ]; then
    echo "Tüm versiyonlar başarıyla derlendi!"
    echo ""
    echo "Çalıştırmak için:"
    echo "  ./run_performance_tests.sh"
else
    echo "Bazı versiyonlar derlenemedi. Hataları kontrol edin."
fi

cd ../../scripts 