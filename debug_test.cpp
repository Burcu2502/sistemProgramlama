#include "../common/password_cracker.h"
#include <iostream>

int main() {
    std::string test_password = "test123";
    size_t target_hash = simple_hash(test_password);
    
    std::cout << "Test şifresi: " << test_password << "\n";
    std::cout << "Hash değeri: " << target_hash << "\n\n";
    
    // İlk birkaç kombinasyonu test et
    std::cout << "İlk 10 kombinasyon:\n";
    for (int i = 0; i < 10; i++) {
        std::string candidate = generate_password(i, 5);
        size_t hash = simple_hash(candidate);
        std::cout << i << ": " << candidate << " -> " << hash << "\n";
    }
    
    // "test123" kaçıncı sırada?
    std::cout << "\ntest123 kaçıncı sırada olabilir?\n";
    
    // 't' harfi charset'te kaçıncı sırada?
    size_t t_index = CHARSET.find('t');
    std::cout << "'t' harfi charset'te " << t_index << ". sırada\n";
    
    // Yaklaşık hesap
    long long combinations_before_t = 0;
    for (int len = 1; len < 7; len++) {
        combinations_before_t += std::pow(CHARSET.length(), len);
    }
    // 7. uzunluk için 't'ye kadar olan kombinasyonlar
    combinations_before_t += t_index * std::pow(CHARSET.length(), 6);
    
    std::cout << "test123'ten önce yaklaşık " << combinations_before_t << " kombinasyon var!\n";
    
    return 0;
} 