#include <algorithm>
#include <iostream>
#include <string>

int main() {
    long long n = 0;
    std::cin >> n;

    long long total = 0;
    long long sansMip = 0;

    for (long long i = 0; i < n; i++) {
        std::string nom, mip;
        long long l = 0, h = 0;
        std::cin >> nom >> l >> h >> mip;

        long long base = l * h * 4;
        long long exact = base;
        long long estime = base;
        long long niveaux = 1;

        if (mip == "OUI") {
            estime = base + base / 3;
            while (l > 1 || h > 1) {
                l = std::max(1LL, l / 2);
                h = std::max(1LL, h / 2);
                exact += l * h * 4;
                niveaux++;
            }
        }

        std::cout << nom << " NIVEAUX " << niveaux << " EXACT " << exact
                  << " ESTIME " << estime << " ECART " << (estime - exact) << '\n';
        total += exact;
        sansMip += base;
    }

    std::cout << "TOTAL " << total << '\n';
    std::cout << "SANS MIPMAPS " << sansMip << '\n';
    if (n == 0) {
        std::cout << "SURCOUT AUCUN\n";
    } else {
        std::cout << "SURCOUT " << ((total - sansMip) * 100 / sansMip) << " %\n";
    }
    return 0;
}
