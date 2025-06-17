#!/bin/bash

# Betiğin, projenin herhangi bir yerinden çalıştırılabilmesi için,
# önce betiğin bulunduğu dizine gider, sonra da bir üst dizine (proje köküne) çıkar.
cd "$(dirname "$0")/.."

# --- Standart Derleme Komutları ---

# Önceki derlemelerden kalan artık dosyaları temizler.
echo "Temizlik yapılıyor..."
rm -f src/version1/password_cracker_v1 src/version1/*.o
rm -f src/version2/password_cracker_v2 src/version2/*.o
rm -f src/version3/password_cracker_v3 src/version3/*.o
rm -f src/version4/password_cracker_v4 src/version4/*.o
rm -f src/common/*.o

# Version 1 Derleme
echo "Version 1 derleniyor..."
g++ -std=c++17 -O2 -Wall -Wextra \
    -o src/version1/password_cracker_v1 \
    src/version1/main.cpp src/common/password_cracker.cpp \
    -lm -pthread

# Version 2 Derleme
echo "Version 2 derleniyor..."
g++ -std=c++17 -O2 -Wall -Wextra \
    -o src/version2/password_cracker_v2 \
    src/version2/main.cpp src/common/password_cracker.cpp \
    -lm -pthread

# Version 3 Derleme
echo "Version 3 derleniyor..."
g++ -std=c++17 -O2 -Wall -Wextra -mavx \
    -o src/version3/password_cracker_v3 \
    src/version3/main.cpp src/common/password_cracker.cpp \
    -lm -pthread

# Version 4 Derleme
echo "Version 4 derleniyor..."
g++ -std=c++17 -O3 -Wall -Wextra -pthread \
    -o src/version4/password_cracker_v4 \
    src/version4/main.cpp src/common/password_cracker.cpp \
    -lm -pthread

echo "Tüm versiyonlar derlendi."

# --- Hata Ayıklama (gdb) ve Profilleme (gprof) için Alternatif Derleme Komutları ---
# Bu komutları kullanmak için yukarıdaki standart derleme komutlarını silip
# buradaki ilgili komutların başındaki '#' işaretini kaldırarak betiği çalıştırabilirsiniz.

# # Version 1 (Hata ayıklama için -g bayrağı ile)
# g++ -std=c++17 -O2 -Wall -Wextra -g \
#     -o src/version1/password_cracker_v1 \
#     src/version1/main.cpp src/common/password_cracker.cpp \
#     -lm -pthread

# # Version 2 (Hata ayıklama için -g bayrağı ile)
# g++ -std=c++17 -O2 -Wall -Wextra -g \
#     -o src/version2/password_cracker_v2 \
#     src/version2/main.cpp src/common/password_cracker.cpp \
#     -lm -pthread

# # Version 3 (Hata ayıklama için -g bayrağı ile)
# g++ -std=c++17 -O2 -Wall -Wextra -mavx -g \
#     -o src/version3/password_cracker_v3 \
#     src/version3/main.cpp src/common/password_cracker.cpp \
#     -lm -pthread

# # Version 4 (Hata ayıklama için -g, profilleme için -pg bayrağı ile)
# g++ -std=c++17 -O3 -Wall -Wextra -pthread -g -pg \
#     -o src/version4/password_cracker_v4 \
#     src/version4/main.cpp src/common/password_cracker.cpp \
#     -lm -pthread -pg 