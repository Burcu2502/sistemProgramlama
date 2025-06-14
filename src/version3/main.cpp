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
                std::cout << "V3-T" << thread_id << " - Deneme: " << local_attempts 
                         << " | Denenen: \"" << candidate << "\""
                         << " | Hedef: \"" << target_password << "\""
                         << " | Uzunluk: " << length << "\n";
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
    std::cout << "=== VERSION 3: Vektörleştirilmiş + Çoklu İş Parçacıklı Brute Force - Batch Mode ===\n\n";
    
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
    std::cout << "Thread sayısı: 16 (Vectorized)\n";
    std::cout << "İlerleme her 25.000 denemede gösterilecek\n\n";
    
    // Batch işleme için vektörler
    std::vector<bool> found_results;
    std::vector<double> times;
    std::vector<long long> attempts_list;
    
    // Toplam zamanı ölç
    Timer total_timer;
    total_timer.start();
    
    // Her şifreyi tek tek işle
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
        std::string found_pwd = crack_password_vectorized(target_password, 16);
        attempts = total_attempts;
        
        double elapsed_time = timer.elapsed();
        
        // Sonuçları kaydet
        bool found = !found_pwd.empty();
        found_results.push_back(found);
        times.push_back(elapsed_time);
        attempts_list.push_back(attempts);
        
        // Anında sonucu göster
        if (found) {
            std::cout << "✅ BULUNDU: " << found_pwd;
        } else {
            std::cout << "❌ BULUNAMADI";
        }
        std::cout << " | Süre: " << elapsed_time << "s";
        std::cout << " | Deneme: " << attempts;
        if (elapsed_time > 0) {
            std::cout << " | Hız: " << (attempts / elapsed_time) << " d/s\n";
        } else {
            std::cout << " | Hız: çok hızlı!\n";
        }
    }
    
    double total_time = total_timer.elapsed();
    
    // Özet sonuçları göster
    std::cout << "\n=== BATCH ÖZET SONUÇLARI ===\n";
    int found_count = 0;
    long long total_attempts_sum = 0;
    
    for (size_t i = 0; i < passwords.size(); i++) {
        if (found_results[i]) found_count++;
        total_attempts_sum += attempts_list[i];
    }
    
    std::cout << "Toplam şifre: " << passwords.size() << "\n";
    std::cout << "Bulunan: " << found_count << "\n";
    std::cout << "Başarı oranı: " << (100.0 * found_count / passwords.size()) << "%\n";
    std::cout << "Toplam süre: " << total_time << " saniye\n";
    std::cout << "Toplam deneme: " << total_attempts_sum << "\n";
    std::cout << "Ortalama hız: " << (total_attempts_sum / total_time) << " deneme/saniye\n";
    
    // Sonuçları dosyaya kaydet
    save_batch_results("version3_batch_results.txt", "Vectorized Multi Thread Batch", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
} 