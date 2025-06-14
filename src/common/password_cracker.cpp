#include "password_cracker.h"
#include <iostream>
#include <fstream>
#include <cmath>

// Daha güçlü hash fonksiyonu - collision'ları minimize etmek için
size_t simple_hash(const std::string& password) {
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
std::string generate_password(long long index, int length) {
    std::string password;
    password.reserve(length);
    
    for (int i = 0; i < length; i++) {
        password += CHARSET[index % CHARSET.length()];
        index /= CHARSET.length();
    }
    
    return password;
}

// Veri seti okuma fonksiyonu
std::vector<std::string> load_passwords_from_file(const std::string& filename) {
    std::vector<std::string> passwords;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Hata: " << filename << " dosyası açılamadı!\n";
        return passwords;
    }
    
    while (std::getline(file, line)) {
        // Boş satırları atla
        if (!line.empty()) {
            // Satır sonundaki boşlukları temizle
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            passwords.push_back(line);
        }
    }
    
    file.close();
    std::cout << "Veri setinden " << passwords.size() << " şifre yüklendi.\n";
    
    return passwords;
}

// Sonuçları kaydetme
void save_results(const std::string& filename, const std::string& version, 
                 const std::string& password, double time, long long attempts) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "=== " << version << " ===" << std::endl;
        file << "Hedef şifre: " << password << std::endl;
        file << "Süre: " << time << " saniye" << std::endl;
        file << "Deneme sayısı: " << attempts << std::endl;
        file << "Hız: " << (attempts / time) << " deneme/saniye" << std::endl;
        file << std::endl;
        file.close();
    }
}

// Batch sonuçları kaydetme
void save_batch_results(const std::string& filename, const std::string& version,
                       const std::vector<std::string>& passwords,
                       const std::vector<bool>& found_results,
                       const std::vector<double>& times,
                       const std::vector<long long>& attempts,
                       double total_time) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "=== " << version << " - BATCH SONUÇLARI ===" << std::endl;
        file << "Toplam süre: " << total_time << " saniye" << std::endl;
        file << "Toplam şifre sayısı: " << passwords.size() << std::endl;
        file << std::endl;
        
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
            file << " | Deneme: " << attempts[i] << std::endl;
            total_attempts += attempts[i];
        }
        
        file << std::endl;
        file << "Başarı oranı: " << found_count << "/" << passwords.size() 
             << " (" << (100.0 * found_count / passwords.size()) << "%)" << std::endl;
        file << "Toplam deneme: " << total_attempts << std::endl;
        file << "Ortalama hız: " << (total_attempts / total_time) << " deneme/saniye" << std::endl;
        
        file.close();
    }
}

// Maksimum uzunluğa kadar toplam kombinasyon sayısını hesapla
long long calculate_total_combinations(int max_length) {
    long long total = 0;
    long long base = CHARSET.length();
    
    for (int len = 1; len <= max_length; len++) {
        total += std::pow(base, len);
    }
    
    return total;
}

