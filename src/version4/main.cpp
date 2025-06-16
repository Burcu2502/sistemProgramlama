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

// V4: Akıllı optimizasyonlu worker thread
void smart_worker_thread(const std::string& target_password, int thread_id, int num_threads) {
    long long local_attempts = 0;
    size_t target_hash = simple_hash(target_password);
    
    // Charset uzunluğunu önceden hesapla
    const size_t charset_len = CHARSET.length();
    
    // String'i tekrar kullan, sürekli allocation yapma
    std::string candidate;
    candidate.reserve(MAX_PASSWORD_LENGTH);
    
    for (int length = 1; length <= MAX_PASSWORD_LENGTH && !password_found.load(std::memory_order_relaxed); length++) {
        long long combinations_for_length = std::pow(charset_len, length);
        
        // İş yükünü thread'ler arasında adil dağıt
        long long chunk_size = combinations_for_length / num_threads;
        long long start = thread_id * chunk_size;
        long long end = (thread_id == num_threads - 1) ? combinations_for_length : start + chunk_size;
        
        for (long long i = start; i < end && !password_found.load(std::memory_order_relaxed); i++) {
            local_attempts++;
            
            // Şifreyi direkt burada üret, fonksiyon çağırma
            candidate.clear();
            candidate.resize(length);
            long long temp = i;
            for (int pos = 0; pos < length; pos++) {
                candidate[pos] = CHARSET[temp % charset_len];
                temp /= charset_len;
            }
            
            // Önce hash karşılaştır, daha hızlı
            size_t candidate_hash = simple_hash(candidate);
            if (candidate_hash == target_hash) {
                // Hash eşleşirse string karşılaştır
                if (candidate == target_password) {
                    std::lock_guard<std::mutex> lock(result_mutex);
                    if (!password_found) {
                        found_password = candidate;
                        password_found = true;
                        std::cout << "Thread " << thread_id << " buldu: " << candidate << "\n";
                    }
                    goto thread_exit;
                }
            }
            
            // İlerleme raporu
            if (local_attempts % 50000 == 0) {
                std::cout << "V4-T" << thread_id << " - Deneme: " << local_attempts 
                         << " | Denenen: \"" << candidate << "\""
                         << " | Hedef: \"" << target_password << "\""
                         << " | Uzunluk: " << length << "\n";
            }
        }
    }
    
    thread_exit:
    total_attempts.fetch_add(local_attempts, std::memory_order_relaxed);
}

// V4: Akıllı şifre kırma fonksiyonu
std::string crack_password_smart(const std::string& target_password, long long& attempts) {
    // Global durumu sıfırla
    password_found = false;
    total_attempts = 0;
    found_password.clear();
    
    // Optimal thread sayısı: fazla thread overhead yaratır
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 8;
    num_threads = std::min(static_cast<int>(std::thread::hardware_concurrency()), 12);
    
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    
    // Thread'leri başlat
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(smart_worker_thread, target_password, i, num_threads);
    }
    
    // Thread'lerin bitmesini bekle
    for (auto& t : threads) {
        t.join();
    }
    
    attempts = total_attempts.load();
    return found_password;
}

int main() {
    std::cout << "=== VERSION 4: AKILLI OPTIMIZASYONLAR ===\n\n";
    
    // Veri setini yükle
    std::string dataset_file = "../../dataset/passwords.txt";
    std::vector<std::string> passwords = load_passwords_from_file(dataset_file);
    
    if (passwords.empty()) {
        std::cout << "Veri seti bos, tek sifre moduna geciliyor...\n";
        passwords.push_back("test");
    }
    
    int num_threads = std::min(static_cast<int>(std::thread::hardware_concurrency()), 12);
    
    std::cout << "Karakter seti: " << CHARSET << "\n";
    std::cout << "Maksimum uzunluk: " << MAX_PASSWORD_LENGTH << "\n";
    std::cout << "Thread sayisi: " << num_threads << "\n";
    std::cout << "Optimizasyonlar: String reuse, inline generation, optimal work distribution\n";
    std::cout << "Ilerleme her 50.000 denemede gosterilecek\n\n";
    
    // Sonuç depolama
    std::vector<bool> found_results;
    std::vector<double> times;
    std::vector<long long> attempts_list;
    
    Timer total_timer;
    total_timer.start();
    
    // Her şifreyi işle
    for (size_t i = 0; i < passwords.size(); i++) {
        std::string target_password = passwords[i];
        size_t target_hash = simple_hash(target_password);
        
        std::cout << "\n--- Sifre " << (i+1) << "/" << passwords.size() 
                 << ": \"" << target_password << "\" ---\n";
        std::cout << "Hedef hash: " << target_hash << "\n";
        
        Timer timer;
        timer.start();
        
        long long attempts = 0;
        std::string found_pwd = crack_password_smart(target_password, attempts);
        
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
            std::cout << " | Hiz: ANINDA!\n";
        }
    }
    
    double total_time = total_timer.elapsed();
    
    // Final sonuçlar
    std::cout << "\n=== AKILLI OPTIMIZASYON SONUCLARI ===\n";
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
    std::cout << "AKILLI HIZ: " << (total_attempts_sum / total_time) << " deneme/saniye\n";
    
    // Sonuçları dosyaya kaydet
    save_batch_results("results_v4.txt", "AKILLI OPTIMIZASYONLAR", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
} 