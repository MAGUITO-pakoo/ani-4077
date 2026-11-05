#include <cmath>
#include <cstdlib>
#include <iostream>

static int versOctet(double v) {
    return static_cast<int>(std::floor(v * 255.0 + 0.5));
}

int main() {
    int n = 0;
    std::cin >> n;

    int identiques = 0;
    int ecartMax = 0;
    int octetMax = 0;

    for (int i = 0; i < n; i++) {
        int c = 0;
        std::cin >> c;

        double x = c / 255.0;
        double l = 0.0;
        if (x <= 0.04045) {
            l = x / 12.92;
        } else {
            l = std::pow((x + 0.055) / 1.055, 2.4);
        }

        int decode = versOctet(l);
        int gamma = versOctet(std::pow(l, 1.0 / 2.2));
        double exact = 0.0;
        if (l <= 0.0031308) {
            exact = l * 12.92;
        } else {
            exact = 1.055 * std::pow(l, 1.0 / 2.4) - 0.055;
        }
        int e = versOctet(exact);
        int ecart = gamma - c;

        std::cout << c << " DECODE " << decode << " GAMMA " << gamma << " EXACT " << e
                  << " ECART " << ecart << '\n';

        if (gamma == c) {
            identiques++;
        }
        if (std::abs(ecart) > ecartMax) {
            ecartMax = std::abs(ecart);
            octetMax = c;
        }
    }

    std::cout << "IDENTIQUES " << identiques << '\n';
    std::cout << "ECART MAX " << ecartMax << " POUR " << octetMax << '\n';
    return 0;
}
