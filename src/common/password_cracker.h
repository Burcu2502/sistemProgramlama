#pragma once

#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>

// Ortak sabitler
const std::string CHARSET = "abcdefghijklmnopqrstuvwxyz0123456789";
const int MAX_PASSWORD_LENGTH = 6;

// Basit hash fonksiyonu (simülasyon için)
size_t simple_hash(const std::string& password);

// Şifre kombinasyonu üreteci
std::string generate_password(long long index, int length);

// Toplam kombinasyon sayısını hesapla
long long calculate_total_combinations(int max_length);

// Zamanı ölç
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    double elapsed() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
};

// Sonuçları dosyaya kaydet
void save_results(const std::string& filename, const std::string& version, 
                 const std::string& password, double time_taken, 
                 long long attempts);

// Veri seti okuma fonksiyonu
std::vector<std::string> load_passwords_from_file(const std::string& filename);

// Batch sonuçları kaydetme
void save_batch_results(const std::string& filename, const std::string& version,
                       const std::vector<std::string>& passwords,
                       const std::vector<bool>& found_results,
                       const std::vector<double>& times,
                       const std::vector<long long>& attempts,
                       double total_time);