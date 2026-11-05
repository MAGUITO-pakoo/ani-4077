#include <algorithm>
#include <iostream>
#include <string>

static long long chaineComplete(long long l, long long h) {
    long long n = 1;
    while (l > 1 || h > 1) {
        l = std::max(1LL, l / 2);
        h = std::max(1LL, h / 2);
        n++;
    }
    return n;
}

int main() {
    long long n = 0;
    std::cin >> n;

    long long noires = 0;
    long long perdus = 0;

    for (long long i = 0; i < n; i++) {
        std::string nom, filtre, niveauxTexte;
        long long l = 0, h = 0;
        std::cin >> nom >> l >> h >> filtre >> niveauxTexte;

        long long complete = chaineComplete(l, h);
        long long niveaux = complete;
        if (niveauxTexte != "GENERE") {
            niveaux = std::min(std::stoll(niveauxTexte), complete);
        }

        if (filtre == "DEFAUT") {
            filtre = "GL_NEAREST_MIPMAP_LINEAR";
        }

        if (filtre.find("MIPMAP") != std::string::npos) {
            if (niveaux < complete) {
                std::cout << nom << " NOIRE MANQUE " << (complete - niveaux) << '\n';
                noires++;
            } else {
                std::cout << nom << " COMPLETE\n";
            }
        } else {
            std::cout << nom << " COMPLETE\n";
            perdus += niveaux - 1;
        }
    }

    std::cout << "NOIRES " << noires << '\n';
    std::cout << "NIVEAUX PERDUS " << perdus << '\n';
    std::cout << "GL_GET_ERROR GL_NO_ERROR\n";
    return 0;
}
