#include "../common/password_cracker.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

using namespace std;

// Global değişkenler
atomic<bool> password_found(false);
atomic<long long> total_attempts(0);
string found_password;
mutex result_mutex;

// Her thread'in çalışacağı fonksiyon
void worker_thread(const string& target_password, int thread_id, int num_threads) {
    long long local_attempts = 0;
    size_t target_hash = simple_hash(target_password);
    
    for (int length = 1; length <= MAX_PASSWORD_LENGTH && !password_found; length++) {
        long long combinations_for_length = pow(CHARSET.length(), length);
        
        // Her thread farklı index'leri kontrol eder
        for (long long i = thread_id; i < combinations_for_length && !password_found; i += num_threads) {
            string candidate = generate_password(i, length);
            local_attempts++;
            
            if (local_attempts % 50000 == 0) {
                cout << "V2-T" << thread_id << " - Deneme: " << local_attempts 
                         << " | Denenen: \"" << candidate << "\""
                         << " | Hedef: \"" << target_password << "\""
                         << " | Uzunluk: " << length << "\n";
            }
            
            // Hash karşılaştırması yap
            if (simple_hash(candidate) == target_hash) {
                // Hash collision kontrolü
                if (candidate == target_password) {
                    lock_guard<mutex> lock(result_mutex);
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
string crack_password_multi_thread(const string& target_password, long long& attempts) {
    // Global değişkenleri sıfırla
    password_found = false;
    total_attempts = 0;
    found_password = "";
    
    const int num_threads = 12;
    vector<thread> threads;
    
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
    cout << "=== VERSION 2: MULTI-THREAD BRUTE FORCE ===\n\n";
    
    // Veri setini yükle
    string dataset_file = "../../dataset/passwords.txt";
    vector<string> passwords = load_passwords_from_file(dataset_file);
    
    if (passwords.empty()) {
        cout << "Veri seti bos, tek sifre moduna geciliyor...\n";
        string single_password = "test";
        passwords.push_back(single_password);
    }
    
    cout << "Karakter seti: " << CHARSET << "\n";
    cout << "Maksimum uzunluk: " << MAX_PASSWORD_LENGTH << "\n";
    cout << "Thread sayisi: 12\n";
    cout << "Ilerleme her 50.000 denemede gosterilecek\n\n";
    
    // Sonuç depolama
    vector<bool> found_results;
    vector<double> times;
    vector<long long> attempts_list;
    
    Timer total_timer;
    total_timer.start();
    
    // Her şifreyi tek tek işle
    for (size_t i = 0; i < passwords.size(); i++) {
        string target_password = passwords[i];
        size_t target_hash = simple_hash(target_password);
        
        cout << "\n--- Sifre " << (i+1) << "/" << passwords.size() 
                 << ": \"" << target_password << "\" ---\n";
        cout << "Hedef hash: " << target_hash << "\n";
        
        Timer timer;
        timer.start();
        
        long long attempts = 0;
        string found_pwd = crack_password_multi_thread(target_password, attempts);
        
        double elapsed_time = timer.elapsed();
        
        // Sonuçları kaydet
        bool found = !found_pwd.empty();
        found_results.push_back(found);
        times.push_back(elapsed_time);
        attempts_list.push_back(attempts);
        
        // Sonucu göster
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
    
    // Özet sonuçlar
    cout << "\n=== MULTI-THREAD SONUCLARI ===\n";
    int found_count = 0;
    long long total_attempts_sum = 0;
    
    for (size_t i = 0; i < passwords.size(); i++) {
        if (found_results[i]) found_count++;
        total_attempts_sum += attempts_list[i];
    }
    
    cout << "Toplam sifre: " << passwords.size() << "\n";
    cout << "Bulunan: " << found_count << "\n";
    cout << "Basari orani: " << (100.0 * found_count / passwords.size()) << "%\n";
    cout << "Toplam sure: " << total_time << " saniye\n";
    cout << "Toplam deneme: " << total_attempts_sum << "\n";
    cout << "Ortalama hiz: " << (total_attempts_sum / total_time) << " deneme/saniye\n";
    
    // Sonuçları dosyaya kaydet
    save_batch_results("results_v2.txt", "MULTI-THREAD", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
} 