#include "password_cracker.h"
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

// Daha güçlü hash fonksiyonu - collision'ları minimize etmek için
size_t simple_hash(const string& password) {
    size_t hash = 0;
    size_t prime1 = 31;
    size_t prime2 = 37;
    size_t prime3 = 41;
    
    for (size_t i = 0; i < password.length(); i++) {
        // Hem karakter hem de pozisyon önemli
        hash = hash * prime1 + (password[i] * prime2) + (i * prime3);
        hash = hash ^ (hash >> 16); // bit mixing
    }
    
    // Final bit mixing
    hash ^= (hash >> 11);
    hash ^= (hash << 7);
    hash ^= (hash >> 17);
    
    return hash;
}

// Şifre kombinasyonu üreteci
string generate_password(long long index, int length) {
    string password;
    password.reserve(length);
    
    for (int i = 0; i < length; i++) {
        password += CHARSET[index % CHARSET.length()];
        index /= CHARSET.length();
    }
    
    return password;
}

// Veri seti okuma fonksiyonu
vector<string> load_passwords_from_file(const string& filename) {
    vector<string> passwords;
    ifstream file(filename);
    string line;
    
    if (!file.is_open()) {
        cerr << "Hata: " << filename << " dosyası açılamadı!\n";
        return passwords;
    }
    
    while (getline(file, line)) {
        // Boş satırları atla
        if (!line.empty()) {
            // Satır sonundaki boşlukları temizle
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            passwords.push_back(line);
        }
    }
    
    file.close();
    cout << "Veri setinden " << passwords.size() << " şifre yüklendi.\n";
    
    return passwords;
}

// Sonuçları kaydetme
void save_results(const string& filename, const string& version, 
                 const string& password, double time, long long attempts) {
    ofstream file(filename, ios::app);
    if (file.is_open()) {
        file << "=== " << version << " ===" << endl;
        file << "Hedef şifre: " << password << endl;
        file << "Süre: " << time << " saniye" << endl;
        file << "Deneme sayısı: " << attempts << endl;
        file << "Hız: " << (attempts / time) << " deneme/saniye" << endl;
        file << endl;
        file.close();
    }
}

// Batch sonuçları kaydetme
void save_batch_results(const string& filename, const string& version,
                       const vector<string>& passwords,
                       const vector<bool>& found_results,
                       const vector<double>& times,
                       const vector<long long>& attempts,
                       double total_time) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "=== " << version << " - BATCH SONUÇLARI ===" << endl;
        file << "Toplam süre: " << total_time << " saniye" << endl;
        file << "Toplam şifre sayısı: " << passwords.size() << endl;
        file << endl;
        
        long long total_attempts = 0;
        int found_count = 0;
        
        for (size_t i = 0; i < passwords.size(); i++) {
            file << "Şifre: " << passwords[i] << " | ";
            if (found_results[i]) {
                file << "BULUNDU";
                found_count++;
            } else {
                file << "BULUNAMADI";
            }
            file << " | Süre: " << times[i] << "s";
            file << " | Deneme: " << attempts[i] << endl;
            total_attempts += attempts[i];
        }
        
        file << endl;
        file << "Başarı oranı: " << found_count << "/" << passwords.size() 
             << " (" << (100.0 * found_count / passwords.size()) << "%)" << endl;
        file << "Toplam deneme: " << total_attempts << endl;
        file << "Ortalama hız: " << (total_attempts / total_time) << " deneme/saniye" << endl;
        
        file.close();
    }
}

// Maksimum uzunluğa kadar toplam kombinasyon sayısını hesapla
long long calculate_total_combinations(int max_length) {
    long long total = 0;
    long long base = CHARSET.length();
    
    for (int len = 1; len <= max_length; len++) {
        total += pow(base, len);
    }
    
    return total;
}

