#include "../common/password_cracker.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <cstring>

// Global değişkenler
std::atomic<bool> password_found(false);
std::atomic<long long> total_attempts(0);
std::string found_password;
std::mutex result_mutex;

// Vectorized hash (compiler otomatik vektörizasyon)
inline uint32_t vectorized_hash(const std::string& str) {
    const char* data = str.c_str();
    size_t len = str.length();
    uint32_t hash = 5381;
    
    // Loop unrolling ile vektörizasyonu teşvik et
    size_t i = 0;
    for (; i + 4 <= len; i += 4) {
        hash = hash * 33 + (unsigned char)data[i];
        hash = hash * 33 + (unsigned char)data[i+1];
        hash = hash * 33 + (unsigned char)data[i+2];
        hash = hash * 33 + (unsigned char)data[i+3];
    }
    
    // Kalan karakterleri işle
    for (; i < len; i++) {
        hash = hash * 33 + (unsigned char)data[i];
    }
    
    return hash;
}

// Vectorized string compare (compiler otomatik vektörizasyon)
inline bool vectorized_compare(const std::string& str1, const std::string& str2) {
    if (str1.length() != str2.length()) return false;
    
    const char* s1 = str1.c_str();
    const char* s2 = str2.c_str();
    size_t len = str1.length();
    
    // Loop unrolling ile vektörizasyonu teşvik et
    size_t i = 0;
    for (; i + 8 <= len; i += 8) {
        if (s1[i] != s2[i] || s1[i+1] != s2[i+1] || 
            s1[i+2] != s2[i+2] || s1[i+3] != s2[i+3] ||
            s1[i+4] != s2[i+4] || s1[i+5] != s2[i+5] ||
            s1[i+6] != s2[i+6] || s1[i+7] != s2[i+7]) {
            return false;
        }
    }
    
    // Kalan karakterleri kontrol et
    for (; i < len; i++) {
        if (s1[i] != s2[i]) return false;
    }
    
    return true;
}

// V3: Multi-threading + Vectorization worker thread
void vectorized_worker_thread(const std::string& target_password, int thread_id, int num_threads) {
    long long local_attempts = 0;
    uint32_t target_hash = vectorized_hash(target_password);
    
    for (int length = 1; length <= MAX_PASSWORD_LENGTH && !password_found; length++) {
        long long combinations_for_length = std::pow(CHARSET.length(), length);
        
        for (long long i = thread_id; i < combinations_for_length && !password_found; i += num_threads) {
            
            // Password generation
            std::string candidate;
            candidate.reserve(MAX_PASSWORD_LENGTH);
            long long temp = i;
            for (int pos = 0; pos < length; pos++) {
                candidate += CHARSET[temp % CHARSET.length()];
                temp /= CHARSET.length();
            }
            
            local_attempts++;
            
            // Vectorized hash karşılaştırması
            if (vectorized_hash(candidate) == target_hash) {
                // Vectorized string karşılaştırması
                if (vectorized_compare(candidate, target_password)) {
                    std::lock_guard<std::mutex> lock(result_mutex);
                    if (!password_found) {
                        password_found = true;
                        found_password = candidate;
                    }
                    goto found_exit;
                }
            }
            
            // İlerleme raporu
            if (local_attempts % 50000 == 0) {
                std::cout << "V3-T" << thread_id << " - Deneme: " << local_attempts 
                         << " | Denenen: \"" << candidate << "\""
                         << " | Hedef: \"" << target_password << "\""
                         << " | Uzunluk: " << length << "\n";
            }
        }
    }
    
    found_exit:
    total_attempts += local_attempts;
}

// V3: Multi-threading + Vectorization şifre kırma
std::string crack_password_vectorized(const std::string& target_password, long long& attempts) {
    // Global değişkenleri sıfırla
    password_found = false;
    total_attempts = 0;
    found_password = "";
    
    const int num_threads = 12;
    
    std::vector<std::thread> threads;
    
    // Thread'leri başlat
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(vectorized_worker_thread, target_password, i, num_threads);
    }
    
    // Thread'lerin bitmesini bekle
    for (auto& t : threads) {
        t.join();
    }
    
    attempts = total_attempts;
    return found_password;
}

int main() {
    std::cout << "=== VERSION 3: MULTI-THREADING + VECTORIZATION ===\n\n";
    
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
    std::cout << "Optimizasyonlar: Multi-threading + Auto-vectorization\n";
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
        uint32_t target_hash = vectorized_hash(target_password);
        
        std::cout << "\n--- Sifre " << (i+1) << "/" << passwords.size() 
                 << ": \"" << target_password << "\" ---\n";
        std::cout << "Hedef hash: " << target_hash << "\n";
        
        Timer timer;
        timer.start();
        
        long long attempts = 0;
        std::string found_pwd = crack_password_vectorized(target_password, attempts);
        
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
    std::cout << "\n=== MULTI-THREADING + VECTORIZATION SONUCLARI ===\n";
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
    save_batch_results("results_v3.txt", "MULTI-THREADING + VECTORIZATION", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
} 