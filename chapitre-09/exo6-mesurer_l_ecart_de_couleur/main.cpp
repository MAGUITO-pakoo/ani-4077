#include <cmath>
#include <iostream>

static int decodeEtEcris(int octet) {
    double x = octet / 255.0;
    double lineaire = 0.0;
    if (x <= 0.04045) {
        lineaire = x / 12.92;
    } else {
        lineaire = std::pow((x + 0.055) / 1.055, 2.4);
    }
    return static_cast<int>(std::floor(lineaire * 255.0 + 0.5));
}

int main() {
    long long n = 0;
    std::cin >> n;

    long long somme[3] = {0, 0, 0};

    for (long long i = 0; i < n; i++) {
        int lu[3];
        int ecrit[3];
        std::cin >> lu[0] >> lu[1] >> lu[2];
        for (int c = 0; c < 3; c++) {
            ecrit[c] = decodeEtEcris(lu[c]);
            somme[c] += ecrit[c] - lu[c];
        }
        std::cout << "LU " << lu[0] << ' ' << lu[1] << ' ' << lu[2] << " ECRIT "
                  << ecrit[0] << ' ' << ecrit[1] << ' ' << ecrit[2] << " ECART "
                  << (ecrit[0] - lu[0]) << ' ' << (ecrit[1] - lu[1]) << ' '
                  << (ecrit[2] - lu[2]) << '\n';
    }

    std::cout << "ECART TOTAL " << somme[0] << ' ' << somme[1] << ' ' << somme[2] << '\n';

    const char* noms[3] = {"ROUGE", "VERT", "BLEU"};
    int pire = 0;
    for (int c = 1; c < 3; c++) {
        if (somme[c] < somme[pire]) {
            pire = c;
        }
    }
    if (somme[0] == 0 && somme[1] == 0 && somme[2] == 0) {
        std::cout << "PLUS TOUCHEE AUCUNE\n";
    } else {
        std::cout << "PLUS TOUCHEE " << noms[pire] << '\n';
    }
    return 0;
}
