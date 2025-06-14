#include "password_cracker.h"
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

// Verilen indekse göre şifre kombinasyonu üret
std::string generate_password(int index, int length) {
    std::string password;
    password.reserve(length);
    
    for (int i = 0; i < length; i++) {
        password += CHARSET[index % CHARSET.length()];
        index /= CHARSET.length();
    }
    
    return password;
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

// Timer sınıfı implementasyonu
void Timer::start() {
    start_time = std::chrono::high_resolution_clock::now();
}

double Timer::elapsed() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    return duration.count() / 1000000.0; // saniye cinsinden
}

void Timer::print_elapsed(const std::string& operation_name) {
    double elapsed_time = elapsed();
    std::cout << operation_name << " süresi: " << elapsed_time << " saniye\n";
}

// Sonuçları dosyaya kaydet
void save_results(const std::string& filename, const std::string& version,
                 const std::string& password, double time_taken,
                 long long attempts) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "Versiyon: " << version << "\n";
        file << "Bulunan şifre: " << password << "\n";
        file << "Süre: " << time_taken << " saniye\n";
        file << "Deneme sayısı: " << attempts << "\n";
        file << "Saniyede deneme: " << (attempts / time_taken) << "\n";
        file << "------------------------\n";
        file.close();
    }
} 