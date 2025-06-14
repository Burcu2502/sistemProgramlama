#include "../common/password_cracker.h"
#include <iostream>
#include <fstream>

// Tek iş parçacıklı brute force fonksiyonu
std::string crack_password_single_thread(const std::string& target_password, long long& attempts) {
    attempts = 0;
    size_t target_hash = simple_hash(target_password);
    
    // Uzunluk 1'den başlayarak maksimum uzunluğa kadar dene
    for (int length = 1; length <= MAX_PASSWORD_LENGTH; length++) {
        std::cout << "Uzunluk " << length << " şifreler deneniyor...\n";
        
        // Bu uzunluk için tüm kombinasyonları dene
        long long combinations_for_length = std::pow(CHARSET.length(), length);
        
        for (long long i = 0; i < combinations_for_length; i++) {
            std::string candidate = generate_password(i, length);
            attempts++;
            
            // Her 25000 denemede bir ilerleme göster
            if (attempts % 25000 == 0) {
                std::cout << "Deneme sayısı: " << attempts 
                         << ", Son denenen: " << candidate 
                         << ", Uzunluk: " << length << "\n";
            }
            
            // Hash karşılaştırması yap - gerçek brute force için
            if (simple_hash(candidate) == target_hash) {
                // Double check - hash collision kontrolü
                if (candidate == target_password) {
                    return candidate;
                } else {
                    std::cout << "Hash collision tespit edildi: " << candidate 
                             << " vs " << target_password << "\n";
                }
            }
        }
    }
    
    return ""; // Bulunamadı
}

int main() {
    std::cout << "=== VERSION 1: Tek İş Parçacıklı Brute Force ===\n\n";
    
    // Test şifresi belirle - demo için orta uzunlukta şifre
    std::string target_password = "test1";
    size_t target_hash = simple_hash(target_password);
    
    std::cout << "Hedef şifre: " << target_password << "\n";
    std::cout << "Hedef hash: " << target_hash << "\n";
    std::cout << "Karakter seti: " << CHARSET << "\n";
    std::cout << "Maksimum uzunluk: " << MAX_PASSWORD_LENGTH << "\n";
    std::cout << "Tek iş parçacığı ile sıralı deneme\n";
    std::cout << "İlerleme her 25.000 denemede gösterilecek\n\n";
    
    // Zamanı ölçmeye başla
    Timer timer;
    timer.start();
    
    long long attempts = 0;
    std::string found_password = crack_password_single_thread(target_password, attempts);
    
    double elapsed_time = timer.elapsed();
    
    // Sonuçları göster
    std::cout << "\n=== SONUÇLAR ===\n";
    if (!found_password.empty()) {
        std::cout << "Şifre bulundu: " << found_password << "\n";
    } else {
        std::cout << "Şifre bulunamadı!\n";
    }
    
    std::cout << "Toplam deneme: " << attempts << "\n";
    std::cout << "Geçen süre: " << elapsed_time << " saniye\n";
    std::cout << "Saniyede deneme: " << (attempts / elapsed_time) << "\n";
    
    // Sonuçları dosyaya kaydet
    save_results("version1_results.txt", "Single Thread", 
                found_password, elapsed_time, attempts);
    
    return 0;
} 