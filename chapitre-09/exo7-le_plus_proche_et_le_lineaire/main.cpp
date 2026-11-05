#include <iostream>
#include <vector>

// Division entière arrondie vers le bas, y compris pour un nombre négatif.
static long long arrondiBas(long long a, long long b) {
    long long q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) {
        q--;
    }
    return q;
}

static long long borne(long long i, long long dernier) {
    if (i < 0) return 0;
    if (i > dernier) return dernier;
    return i;
}

int main() {
    long long l = 1, h = 1, q = 0;
    std::cin >> l >> h >> q;

    // texels[j][i] : j = 0 est la ligne du bas, là où v vaut 0.
    std::vector<std::vector<long long>> texels(static_cast<size_t>(h),
                                               std::vector<long long>(static_cast<size_t>(l)));
    for (auto& ligne : texels) {
        for (auto& t : ligne) {
            std::cin >> t;
        }
    }

    auto lire = [&](long long i, long long j) {
        return texels[static_cast<size_t>(borne(j, h - 1))][static_cast<size_t>(borne(i, l - 1))];
    };

    long long surs = 0, frontieres = 0;

    for (long long k = 0; k < q; k++) {
        long long u = 0, v = 0;
        std::cin >> u >> v;

        long long a = lire(arrondiBas(u * l, 1000), arrondiBas(v * h, 1000));

        // X et Y comptent x et y en deux-millièmes de texel.
        long long bx = 2 * u * l - 1000;
        long long by = 2 * v * h - 1000;
        long long i0 = arrondiBas(bx, 2000);
        long long j0 = arrondiBas(by, 2000);
        long long fx = bx - i0 * 2000;
        long long fy = by - j0 * 2000;

        long long num = lire(i0, j0) * (2000 - fx) * (2000 - fy) +
                        lire(i0 + 1, j0) * fx * (2000 - fy) +
                        lire(i0, j0 + 1) * (2000 - fx) * fy +
                        lire(i0 + 1, j0 + 1) * fx * fy;
        long long den = 2000LL * 2000LL;
        long long b = arrondiBas(2 * num + den, 2 * den);

        bool sur = (a == b);
        std::cout << u << ' ' << v << " PROCHE " << a << " LINEAIRE " << b << ' '
                  << (sur ? "SUR" : "FRONTIERE") << '\n';
        if (sur) {
            surs++;
        } else {
            frontieres++;
        }
    }

    std::cout << "SURS " << surs << '\n';
    std::cout << "FRONTIERES " << frontieres << '\n';
    return 0;
}
