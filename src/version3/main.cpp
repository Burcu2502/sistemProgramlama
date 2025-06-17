#include "../common/password_cracker.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <cstring>
#include <immintrin.h> // SIMD komutları için

using namespace std;

// Global değişkenler
atomic<bool> password_found(false);
atomic<long long> total_attempts(0);
string found_password;
mutex result_mutex;

// Vektörize edilmiş string karşılaştırma fonksiyonu
// 16 byte'lık bloklar halinde SIMD komutlarıyla karşılaştırma yapar
inline bool vectorized_string_compare(const string& s1, const string& s2) {
    if (s1.length() != s2.length()) {
        return false;
    }

    const char* p1 = s1.c_str();
    const char* p2 = s2.c_str();
    size_t len = s1.length();

    // 16'şar byte'lık bloklarla karşılaştır
    size_t num_blocks = len / 16;
    for (size_t i = 0; i < num_blocks; ++i) {
        __m128i block1 = _mm_loadu_si128((const __m128i*)(p1 + i * 16));
        __m128i block2 = _mm_loadu_si128((const __m128i*)(p2 + i * 16));
        __m128i comparison_result = _mm_cmpeq_epi8(block1, block2);
        
        // Eğer 16 byte'ın tamamı eşit değilse (mask 0xFFFF olmazsa), false dön
        if (_mm_movemask_epi8(comparison_result) != 0xFFFF) {
            return false;
        }
    }

    // Geriye kalan byte'ları normal şekilde karşılaştır
    size_t remaining_bytes = len % 16;
    if (remaining_bytes > 0) {
        return memcmp(p1 + num_blocks * 16, p2 + num_blocks * 16, remaining_bytes) == 0;
    }

    return true;
}

// V3: Multi-threading + Manuel Vectorization worker thread
void vectorized_worker_thread(const string& target_password, int thread_id, int num_threads) {
    long long local_attempts = 0;
    size_t target_hash = simple_hash(target_password);
    
    string candidate;
    candidate.reserve(MAX_PASSWORD_LENGTH);

    for (int length = 1; length <= MAX_PASSWORD_LENGTH && !password_found; ++length) {
        long long combinations_for_length = 1;
        for(int p = 0; p < length; ++p) combinations_for_length *= CHARSET.length();

        for (long long i = thread_id; i < combinations_for_length && !password_found; i += num_threads) {
            candidate.clear();
            long long temp = i;
            
            candidate.resize(length);
            for (int pos = 0; pos < length; ++pos) {
                candidate[pos] = CHARSET[temp % CHARSET.length()];
                temp /= CHARSET.length();
            }
            
            local_attempts++;
            
            // Önce hash ile hızlı kontrol
            if (simple_hash(candidate) == target_hash) {
                // Hash eşleşirse, vektörize edilmiş fonksiyon ile tam karşılaştırma yap
                if (vectorized_string_compare(candidate, target_password)) {
                    lock_guard<mutex> lock(result_mutex);
                    if (!password_found) {
                        password_found = true;
                        found_password = candidate;
                    }
                    goto found_exit;
                }
            }
            
            if (local_attempts % 500000 == 0) {
                cout << "V3-T" << thread_id << " - Deneme: " << local_attempts 
                         << " | Denenen: \"" << candidate << "\""
                         << " | Hedef: \"" << target_password << "\""
                         << " | Uzunluk: " << length << "\n";
            }
        }
    }
    
found_exit:
    total_attempts += local_attempts;
}

// V3: Multi-threading + Manuel Vectorization ile şifre kırma
string crack_password_vectorized(const string& target_password, long long& attempts) {
    password_found = false;
    total_attempts = 0;
    found_password.clear();
    
    const int num_threads = 12;
    vector<thread> threads;
    threads.reserve(num_threads);
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(vectorized_worker_thread, cref(target_password), i, num_threads);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    attempts = total_attempts.load();
    return found_password;
}

int main() {
    cout << "=== VERSION 3: MULTI-THREADING + MANUAL VECTORIZATION ===\n\n";
    
    string dataset_file = "../../dataset/passwords.txt";
    auto passwords = load_passwords_from_file(dataset_file);
    
    if (passwords.empty()) {
        cout << "Veri seti bos, tek sifre moduna geciliyor...\n";
        passwords.push_back("test");
    }
    
    cout << "Karakter seti: " << CHARSET << "\n";
    cout << "Maksimum uzunluk: " << MAX_PASSWORD_LENGTH << "\n";
    cout << "Thread sayisi: 12\n";
    cout << "Optimizasyonlar: Multi-threading + Manual SIMD Vectorization (SSE/AVX)\n";
    cout << "Ilerleme her 500.000 denemede gosterilecek\n\n";
    
    vector<bool> found_results;
    vector<double> times;
    vector<long long> attempts_list;
    
    Timer total_timer;
    total_timer.start();
    
    for (size_t i = 0; i < passwords.size(); ++i) {
        const auto& target_password = passwords[i];
        size_t target_hash = simple_hash(target_password);
        
        cout << "\n--- Sifre " << (i+1) << "/" << passwords.size() 
                 << ": \"" << target_password << "\" ---\n";
        cout << "Hedef hash: " << target_hash << "\n";
        
        Timer timer;
        timer.start();
        
        long long attempts = 0;
        string found_pwd = crack_password_vectorized(target_password, attempts);
        
        double elapsed_time = timer.elapsed();
        
        bool found = !found_pwd.empty();
        found_results.push_back(found);
        times.push_back(elapsed_time);
        attempts_list.push_back(attempts);
        
        if (found) {
            cout << "BULUNDU: " << found_pwd;
        } else {
            cout << "BULUNAMADI";
        }
        cout << " | Sure: " << elapsed_time << "s";
        cout << " | Deneme: " << attempts;
        if (elapsed_time > 0) {
            cout << " | Hiz: " << (attempts / elapsed_time) << " d/s\n";
        } else {
            cout << " | Hiz: cok hizli!\n";
        }
    }
    
    double total_time = total_timer.elapsed();
    
    cout << "\n=== MANUAL VECTORIZATION SONUCLARI ===\n";
    int found_count = 0;
    long long total_attempts_sum = 0;
    
    for (size_t i = 0; i < passwords.size(); ++i) {
        if (found_results[i]) found_count++;
        total_attempts_sum += attempts_list[i];
    }
    
    cout << "Toplam sifre: " << passwords.size() << "\n";
    cout << "Bulunan: " << found_count << "\n";
    cout << "Basari orani: " << (100.0 * found_count / passwords.size()) << "%\n";
    cout << "Toplam sure: " << total_time << " saniye\n";
    cout << "Toplam deneme: " << total_attempts_sum << "\n";
    cout << "Ortalama hiz: " << (total_attempts_sum / total_time) << " deneme/saniye\n";
    
    save_batch_results("results_v3.txt", "MULTI-THREADING + MANUAL VECTORIZATION", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
}