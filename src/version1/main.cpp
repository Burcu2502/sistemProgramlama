#include "../common/password_cracker.h"
#include <iostream>
#include <fstream>

using namespace std;

// Tek thread brute force fonksiyonu
string crack_password_single_thread(const string& target_password, long long& attempts) {
    attempts = 0;
    size_t target_hash = simple_hash(target_password);
    
    // Uzunluk 1'den başlayarak maksimum uzunluğa kadar dene
    for (int length = 1; length <= MAX_PASSWORD_LENGTH; length++) {
        cout << "Uzunluk " << length << " sifreler deneniyor...\n";
        
        // Bu uzunluk için tüm kombinasyonları dene
        long long combinations_for_length = pow(CHARSET.length(), length);
        
        for (long long i = 0; i < combinations_for_length; i++) {
            string candidate = generate_password(i, length);
            attempts++;
            
            // Her 50000 denemede bir ilerleme göster
            if (attempts % 50000 == 0) {
                cout << "V1 - Deneme: " << attempts 
                         << " | Denenen: \"" << candidate << "\""
                         << " | Hedef: \"" << target_password << "\""
                         << " | Uzunluk: " << length << "\n";
            }
            
            // Hash karşılaştırması yap
            if (simple_hash(candidate) == target_hash) {
                // Hash collision kontrolü
                if (candidate == target_password) {
                    return candidate;
                } else {
                    cout << "Hash collision tespit edildi: " << candidate 
                             << " vs " << target_password << "\n";
                }
            }
        }
    }
    
    return "";
}

int main() {
    cout << "=== VERSION 1: TEK THREAD BRUTE FORCE ===\n\n";
    
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
    cout << "Tek thread ile sirali deneme\n";
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
        string found_password = crack_password_single_thread(target_password, attempts);
        
        double elapsed_time = timer.elapsed();
        
        // Sonuçları kaydet
        bool found = !found_password.empty();
        found_results.push_back(found);
        times.push_back(elapsed_time);
        attempts_list.push_back(attempts);
        
        // Sonucu göster
        if (found) {
            cout << "BULUNDU: " << found_password;
        } else {
            cout << "BULUNAMADI";
        }
        cout << " | Sure: " << elapsed_time << "s";
        cout << " | Deneme: " << attempts;
        cout << " | Hiz: " << (attempts / elapsed_time) << " d/s\n";
    }
    
    double total_time = total_timer.elapsed();
    
    // Özet sonuçlar
    cout << "\n=== TEK THREAD SONUCLARI ===\n";
    int found_count = 0;
    long long total_attempts = 0;
    
    for (size_t i = 0; i < passwords.size(); i++) {
        if (found_results[i]) found_count++;
        total_attempts += attempts_list[i];
    }
    
    cout << "Toplam sifre: " << passwords.size() << "\n";
    cout << "Bulunan: " << found_count << "\n";
    cout << "Basari orani: " << (100.0 * found_count / passwords.size()) << "%\n";
    cout << "Toplam sure: " << total_time << " saniye\n";
    cout << "Toplam deneme: " << total_attempts << "\n";
    cout << "Ortalama hiz: " << (total_attempts / total_time) << " deneme/saniye\n";
    
    // Sonuçları dosyaya kaydet
    save_batch_results("results_v1.txt", "TEK THREAD", 
                      passwords, found_results, times, attempts_list, total_time);
    
    return 0;
} 