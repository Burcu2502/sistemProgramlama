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
            
            // Her 50000 denemede bir ilerleme göster (daha az spam)
            if (attempts % 50000 == 0) {
                std::cout << "V1 - Deneme: " << attempts 
                         << " | Denenen: \"" << candidate << "\""
                         << " | Hedef: \"" << target_password << "\""
                         << " | Uzunluk: " << length << "\n";
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
    std::cout << "=== VERSION 1: Tek İş Parçacıklı Brute Force - Batch Mode ===\n\n";
    
    // Veri setini yükle
    std::string dataset_file = "../../dataset/passwords.txt";
    std::vector<std::string> passwords = load_passwords_from_file(dataset_file);
    
    if (passwords.empty()) {
        std::cout << "Veri seti boş, tek şifre moduna geçiliyor...\n";
        std::string single_password = "test";
        passwords.push_back(single_password);
    }
    
    std::cout << "Karakter seti: " << CHARSET << "\n";
    std::cout << "Maksimum uzunluk: " << MAX_PASSWORD_LENGTH << "\n";
    std::cout << "Tek iş parçacığı ile sıralı deneme\n";
    std::cout << "İlerleme her 50.000 denemede gösterilecek\n\n";
    
    // Batch işleme için vektörler
    std::vector<bool> found_results;
    std::vector<double> times;
    std::vector<long long> attempts_list;
    
    // Toplam zamanı ölç
    Timer total_timer;
    total_timer.start();
    
    // Her şifreli tek tek işle
    for (size_t i = 0; i < passwords.size(); i++) {
        std::string target_password = passwords[i];
        size_t target_hash = simple_hash(target_password);
        
        std::cout << "\n--- Şifre " << (i+1) << "/" << passwords.size() 
                 << ": \"" << target_password << "\" ---\n";
        std::cout << "Hedef hash: " << target_hash << "\n";
        
        // Bu şifre için zamanı ölç
        Timer timer;
        timer.start();
        
        long long attempts = 0;
        std::string found_password = crack_password_single_thread(target_password, attempts);
        
        double elapsed_time = timer.elapsed();
        
        // Sonuçları kaydet
        bool found = !found_password.empty();
        found_results.push_back(found);
        times.push_back(elapsed_time);
        attempts_list.push_back(attempts);
        
        // Anında sonucu göster
        if (found) {
            std::cout << "✅ BULUNDU: " << found_password;
        } else {
            std::cout << "❌ BULUNAMADI";
        }
        std::cout << " | Süre: " << elapsed_time << "s";
        std::cout << " | Deneme: " << attempts;
        std::cout << " | Hız: " << (attempts / elapsed_time) << " d/s\n";
    }
    
    double total_time = total_timer.elapsed();
    
    // Özet sonuçları göster
    std::cout << "\n=== BATCH ÖZET SONUÇLARI ===\n";
    int found_count = 0;
    long long total_attempts = 0;
    
    for (size_t i = 0; i < passwords.size(); i++) {
        if (found_results[i]) found_count++;
        total_attempts += attempts_list[i];
    }
    
    std::cout << "Toplam şifre: " << passwords.size() << "\n";
    std::cout << "Bulunan: " << found_count << "\n";
    std::cout << "Başarı oranı: " << (100.0 * found_count / passwords.size()) << "%\n";
    std::cout << "Toplam süre: " << total_time << " saniye\n";
    std::cout << "Toplam deneme: " << total_attempts << "\n";
    std::cout << "Ortalama hız: " << (total_attempts / total_time) << " deneme/saniye\n";
    
    // Sonuçları dosyaya kaydet
    save_batch_results("version1_batch_results.txt", "Single Thread Batch", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
} 