#pragma once

#include <string>
#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>

using namespace std;

// Ortak sabitler
const string CHARSET = "abcdefghijklmnopqrstuvwxyz0123456789";
//const string CHARSET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}|;:',.<>/?`~";

const int MAX_PASSWORD_LENGTH = 6;

// Basit hash fonksiyonu (simülasyon için)
size_t simple_hash(const string& password);

// Şifre kombinasyonu üreteci
string generate_password(long long index, int length);

// Toplam kombinasyon sayısını hesapla
long long calculate_total_combinations(int max_length);

// Zamanı ölç
class Timer {
private:
    chrono::high_resolution_clock::time_point start_time;
    
public:
    void start() {
        start_time = chrono::high_resolution_clock::now();
    }
    
    double elapsed() {
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
};

// Sonuçları dosyaya kaydet
void save_results(const string& filename, const string& version, 
                 const string& password, double time_taken, 
                 long long attempts);

// Veri seti okuma fonksiyonu
vector<string> load_passwords_from_file(const string& filename);

// Batch sonuçları kaydetme
void save_batch_results(const string& filename, const string& version,
                       const vector<string>& passwords,
                       const vector<bool>& found_results,
                       const vector<double>& times,
                       const vector<long long>& attempts,
                       double total_time);