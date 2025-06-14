#include "../common/password_cracker.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

// Global değişkenler
std::atomic<bool> password_found(false);
std::atomic<long long> total_attempts(0);
std::string found_password;
std::mutex result_mutex;

// Her thread'in çalışacağı fonksiyon
void worker_thread(const std::string& target_password, int thread_id, int num_threads) {
    long long local_attempts = 0;
    size_t target_hash = simple_hash(target_password);
    
    for (int length = 1; length <= MAX_PASSWORD_LENGTH && !password_found; length++) {
        long long combinations_for_length = std::pow(CHARSET.length(), length);
        
        // Her thread'in hangi index'leri kontrol edeceği
        for (long long i = thread_id; i < combinations_for_length && !password_found; i += num_threads) {
            std::string candidate = generate_password(i, length);
            local_attempts++;
            
            if (local_attempts % 50000 == 0) {
                std::cout << "V2-T" << thread_id << " - Deneme: " << local_attempts 
                         << " | Denenen: \"" << candidate << "\""
                         << " | Hedef: \"" << target_password << "\""
                         << " | Uzunluk: " << length << "\n";
            }
            
            // Hash karşılaştırması yap
            if (simple_hash(candidate) == target_hash) {
                // Double check - hash collision kontrolü
                if (candidate == target_password) {
                    std::lock_guard<std::mutex> lock(result_mutex);
                    if (!password_found) {
                        password_found = true;
                        found_password = candidate;
                    }
                    break;
                }
            }
        }
    }
    
    total_attempts += local_attempts;
}

// Tek şifre kırma fonksiyonu
std::string crack_password_multi_thread(const std::string& target_password, long long& attempts) {
    // Global değişkenleri sıfırla
    password_found = false;
    total_attempts = 0;
    found_password = "";
    
    const int num_threads = 12;
    std::vector<std::thread> threads;
    
    // Thread'leri başlat
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(worker_thread, target_password, i, num_threads);
    }
    
    // Thread'lerin bitmesini bekle
    for (auto& t : threads) {
        t.join();
    }
    
    attempts = total_attempts;
    return found_password;
}

int main() {
    std::cout << "=== VERSION 2: Çoklu İş Parçacıklı Brute Force - Batch Mode ===\n\n";
    
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
    std::cout << "Thread sayısı: 12\n";
    std::cout << "İlerleme her 50.000 denemede gösterilecek\n\n";
    
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
        std::string found_pwd = crack_password_multi_thread(target_password, attempts);
        
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
    save_batch_results("version2_batch_results.txt", "Multi Thread Batch", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
} 