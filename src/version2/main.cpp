#include "../common/password_cracker.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

// Global değişkenler - thread'ler arası paylaşım için
std::atomic<bool> password_found(false);
std::string found_password;
std::mutex result_mutex;
std::atomic<long long> total_attempts(0);

// Her thread'in çalışacağı fonksiyon
void worker_thread(const std::string& target_password, long long start_index, 
                  long long end_index, int thread_id) {
    long long local_attempts = 0;
    size_t target_hash = simple_hash(target_password);
    
    for (int length = 1; length <= MAX_PASSWORD_LENGTH && !password_found; length++) {
        long long combinations_for_length = std::pow(CHARSET.length(), length);
        
        // Bu thread'e ait aralığı hesapla
        long long thread_start = (start_index * combinations_for_length) / 100;
        long long thread_end = (end_index * combinations_for_length) / 100;
        
        for (long long i = thread_start; i < thread_end && !password_found; i++) {
            std::string candidate = generate_password(i, length);
            local_attempts++;
            
            // Her 25000 denemede bir ilerleme göster
            if (local_attempts % 25000 == 0) {
                std::cout << "Thread " << thread_id << " - Deneme: " 
                         << local_attempts << ", Son: " << candidate 
                         << ", Uzunluk: " << length << "\n";
            }
            
            // Hash karşılaştırması yap - gerçek brute force için
            if (simple_hash(candidate) == target_hash) {
                // Double check - hash collision kontrolü
                if (candidate == target_password) {
                    // Güvenli şekilde sonucu kaydet
                    std::lock_guard<std::mutex> lock(result_mutex);
                    if (!password_found) {
                        found_password = candidate;
                        password_found = true;
                        std::cout << "Thread " << thread_id << " şifreyi buldu: " 
                                 << candidate << "\n";
                    }
                    break;
                } else {
                    std::cout << "Thread " << thread_id << " hash collision: " 
                             << candidate << " vs " << target_password << "\n";
                }
            }
        }
    }
    
    // Toplam deneme sayısını güncelle
    total_attempts += local_attempts;
}

// Çoklu iş parçacıklı brute force fonksiyonu
std::string crack_password_multi_thread(const std::string& target_password, int num_threads) {
    // Global değişkenleri sıfırla
    password_found = false;
    found_password = "";
    total_attempts = 0;
    
    std::vector<std::thread> threads;
    
    std::cout << "Toplam " << num_threads << " thread başlatılıyor...\n";
    std::cout << "Her thread farklı aralıkta paralel çalışacak\n";
    std::cout << "İlerleme her 25.000 denemede gösterilecek\n\n";
    
    // Thread'leri başlat
    for (int i = 0; i < num_threads; i++) {
        long long start_range = (i * 100) / num_threads;
        long long end_range = ((i + 1) * 100) / num_threads;
        
        threads.emplace_back(worker_thread, target_password, 
                           start_range, end_range, i);
    }
    
    // Tüm thread'lerin bitmesini bekle
    for (auto& t : threads) {
        t.join();
    }
    
    return found_password;
}

int main() {
    std::cout << "=== VERSION 2: Çoklu İş Parçacıklı Brute Force ===\n\n";
    
    // Test şifresi belirle
    std::string target_password = "test1";
    size_t target_hash = simple_hash(target_password);
    
    // Sistem işlemci sayısını al
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4; // varsayılan
    
    std::cout << "Hedef şifre: " << target_password << "\n";
    std::cout << "Hedef hash: " << target_hash << "\n";
    std::cout << "Kullanılacak thread sayısı: " << num_threads << "\n";
    std::cout << "Karakter seti: " << CHARSET << "\n";
    std::cout << "Maksimum uzunluk: " << MAX_PASSWORD_LENGTH << "\n\n";
    
    // Zamanı ölçmeye başla
    Timer timer;
    timer.start();
    
    std::string result = crack_password_multi_thread(target_password, num_threads);
    
    double elapsed_time = timer.elapsed();
    
    // Sonuçları göster
    std::cout << "\n=== SONUÇLAR ===\n";
    if (!result.empty()) {
        std::cout << "Şifre bulundu: " << result << "\n";
    } else {
        std::cout << "Şifre bulunamadı!\n";
    }
    
    std::cout << "Toplam deneme: " << total_attempts.load() << "\n";
    std::cout << "Geçen süre: " << elapsed_time << " saniye\n";
    std::cout << "Saniyede deneme: " << (total_attempts.load() / elapsed_time) << "\n";
    std::cout << "Kullanılan thread sayısı: " << num_threads << "\n";
    
    // Sonuçları dosyaya kaydet
    save_results("version2_results.txt", "Multi Thread", 
                result, elapsed_time, total_attempts.load());
    
    return 0;
} 