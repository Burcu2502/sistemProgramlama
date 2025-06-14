#ifndef PASSWORD_CRACKER_H
#define PASSWORD_CRACKER_H

#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>

// Ortak sabitler
const std::string CHARSET = "abcdefghijklmnopqrstuvwxyz0123456789";
const int MAX_PASSWORD_LENGTH = 8;

// Basit hash fonksiyonu (simülasyon için)
size_t simple_hash(const std::string& password);

// Şifre kombinasyonu üreteci
std::string generate_password(int index, int length);

// Toplam kombinasyon sayısını hesapla
long long calculate_total_combinations(int max_length);

// Zamanı ölç
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    
public:
    void start();
    double elapsed();
    void print_elapsed(const std::string& operation_name);
};

// Sonuçları dosyaya kaydet
void save_results(const std::string& filename, const std::string& version, 
                 const std::string& password, double time_taken, 
                 long long attempts);

#endif 