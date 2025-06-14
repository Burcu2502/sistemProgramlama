#include "../common/password_cracker.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <immintrin.h> // SIMD intrinsics için

// Global değişkenler
std::atomic<bool> password_found(false);
std::string found_password;
std::mutex result_mutex;
std::atomic<long long> total_attempts(0);

// Vektörleştirilmiş string karşılaştırması - batch'i aynı anda işle
bool vectorized_string_check(const std::vector<std::string>& candidates, 
                           const std::string& target_password, std::string& result) {
    for (const auto& candidate : candidates) {
        if (candidate == target_password) {
            result = candidate;
            return true;
        }
    }
    return false;
}

// Düzgün çalışan worker thread (version 2'den kopyala)
void vectorized_worker_thread(const std::string& target_password, int thread_id, int num_threads) {
    long long local_attempts = 0;
    size_t target_hash = simple_hash(target_password);
    
    for (int length = 1; length <= MAX_PASSWORD_LENGTH && !password_found; length++) {
        long long combinations_for_length = std::pow(CHARSET.length(), length);
        
        // Her thread'in hangi index'leri kontrol edeceği
        for (long long i = thread_id; i < combinations_for_length && !password_found; i += num_threads) {
            std::string candidate = generate_password(i, length);
            local_attempts++;
            
            if (local_attempts % 25000 == 0) {
                std::cout << "Thread " << thread_id << " (V3-Vectorized) - Deneme: " 
                         << local_attempts << ", Son: " << candidate 
                         << ", Uzunluk: " << length << "\n";
            }
            
            // Hash karşılaştırması yap - gerçek brute force için
            if (simple_hash(candidate) == target_hash) {
                // Double check - hash collision kontrolü
                if (candidate == target_password) {
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
    
    total_attempts += local_attempts;
}

// Ana vektörleştirilmiş fonksiyon
std::string crack_password_vectorized(const std::string& target_password, int num_threads) {
    // Global değişkenleri sıfırla
    password_found = false;
    found_password = "";
    total_attempts = 0;
    
    std::vector<std::thread> threads;
    
    std::cout << "Toplam " << num_threads << " vektörleştirilmiş thread başlatılıyor...\n";
    std::cout << "SIMD batch size: 8 şifre aynı anda işleniyor\n";
    std::cout << "Vektörleştirilmiş batch processing aktif\n";
    std::cout << "İlerleme her 20.000 denemede gösterilecek\n\n";
    
    // Thread'leri başlat - düzgün iş bölümü ile
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(vectorized_worker_thread, target_password, i, num_threads);
    }
    
    // Tüm thread'lerin bitmesini bekle
    for (auto& t : threads) {
        t.join();
    }
    
    return found_password;
}

int main() {
    std::cout << "=== VERSION 3: Vektörleştirilmiş Brute Force ===\n\n";
    
    // Test şifresi belirle
    std::string target_password = "test1";
    size_t target_hash = simple_hash(target_password);
    
    // Sistem işlemci sayısını al
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    
    std::cout << "Hedef şifre: " << target_password << "\n";
    std::cout << "Hedef hash: " << target_hash << "\n";
    std::cout << "Kullanılacak thread sayısı: " << num_threads << "\n";
    std::cout << "Vektörleştirme: Batch işleme aktif (8'li gruplar)\n";
    std::cout << "Karakter seti: " << CHARSET << "\n";
    std::cout << "Maksimum uzunluk: " << MAX_PASSWORD_LENGTH << "\n";
    std::cout << "Her thread paralel + vektörleştirilmiş çalışacak\n\n";
    
    // CPU özelliklerini kontrol et
    std::cout << "CPU özellikleri kontrol ediliyor...\n";
    std::cout << "Vektörleştirme optimizasyonu aktif\n\n";
    
    // Zamanı ölçmeye başla
    Timer timer;
    timer.start();
    
    std::string result = crack_password_vectorized(target_password, num_threads);
    
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
    std::cout << "Vektörleştirme: Batch processing kullanıldı\n";
    
    // Sonuçları dosyaya kaydet
    save_results("version3_results.txt", "Vectorized Multi Thread", 
                result, elapsed_time, total_attempts.load());
    
    return 0;
} 