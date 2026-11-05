#include <iostream>

int main() {
    long long t = 1, m = 0;
    std::cin >> t >> m;

    long long niveauMax = 0;
    while ((t >> niveauMax) > 1) {
        niveauMax++;
    }

    long long scintillent = 0;

    for (long long i = 0; i < m; i++) {
        long long s = 1;
        std::cin >> s;

        long long d = 0;
        long long k = 1;
        if (s < t) {
            while (d < niveauMax && (t >> (d + 1)) >= s) {
                d++;
            }
            long long r = t / s;
            k = r * r;
        }
        long long cote = t >> d;

        std::cout << "ECRAN " << s << " NIVEAU " << d << " COTE " << cote
                  << " UN SUR " << k << '\n';
        if (k > 1) {
            scintillent++;
        }
    }

    std::cout << "NIVEAU MAX " << niveauMax << '\n';
    std::cout << "SCINTILLENT " << scintillent << '\n';
    return 0;
}
