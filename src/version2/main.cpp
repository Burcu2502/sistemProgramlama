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
        
        // Her thread farklı index'leri kontrol eder
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
                // Hash collision kontrolü
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

// Multi-thread şifre kırma fonksiyonu
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
    std::cout << "=== VERSION 2: MULTI-THREAD BRUTE FORCE ===\n\n";
    
    // Veri setini yükle
    std::string dataset_file = "../../dataset/passwords.txt";
    std::vector<std::string> passwords = load_passwords_from_file(dataset_file);
    
    if (passwords.empty()) {
        std::cout << "Veri seti bos, tek sifre moduna geciliyor...\n";
        std::string single_password = "test";
        passwords.push_back(single_password);
    }
    
    std::cout << "Karakter seti: " << CHARSET << "\n";
    std::cout << "Maksimum uzunluk: " << MAX_PASSWORD_LENGTH << "\n";
    std::cout << "Thread sayisi: 12\n";
    std::cout << "Ilerleme her 50.000 denemede gosterilecek\n\n";
    
    // Sonuç depolama
    std::vector<bool> found_results;
    std::vector<double> times;
    std::vector<long long> attempts_list;
    
    Timer total_timer;
    total_timer.start();
    
    // Her şifreyi tek tek işle
    for (size_t i = 0; i < passwords.size(); i++) {
        std::string target_password = passwords[i];
        size_t target_hash = simple_hash(target_password);
        
        std::cout << "\n--- Sifre " << (i+1) << "/" << passwords.size() 
                 << ": \"" << target_password << "\" ---\n";
        std::cout << "Hedef hash: " << target_hash << "\n";
        
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
        
        // Sonucu göster
        if (found) {
            std::cout << "BULUNDU: " << found_pwd;
        } else {
            std::cout << "BULUNAMADI";
        }
        std::cout << " | Sure: " << elapsed_time << "s";
        std::cout << " | Deneme: " << attempts;
        if (elapsed_time > 0) {
            std::cout << " | Hiz: " << (attempts / elapsed_time) << " d/s\n";
        } else {
            std::cout << " | Hiz: cok hizli!\n";
        }
    }
    
    double total_time = total_timer.elapsed();
    
    // Özet sonuçlar
    std::cout << "\n=== MULTI-THREAD SONUCLARI ===\n";
    int found_count = 0;
    long long total_attempts_sum = 0;
    
    for (size_t i = 0; i < passwords.size(); i++) {
        if (found_results[i]) found_count++;
        total_attempts_sum += attempts_list[i];
    }
    
    std::cout << "Toplam sifre: " << passwords.size() << "\n";
    std::cout << "Bulunan: " << found_count << "\n";
    std::cout << "Basari orani: " << (100.0 * found_count / passwords.size()) << "%\n";
    std::cout << "Toplam sure: " << total_time << " saniye\n";
    std::cout << "Toplam deneme: " << total_attempts_sum << "\n";
    std::cout << "Ortalama hiz: " << (total_attempts_sum / total_time) << " deneme/saniye\n";
    
    // Sonuçları dosyaya kaydet
    save_batch_results("results_v2.txt", "MULTI-THREAD", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
} 