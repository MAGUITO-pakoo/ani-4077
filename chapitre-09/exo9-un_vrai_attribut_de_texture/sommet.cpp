#include <iostream>
#include <string>
#include <vector>

int main() {
    int k = 0;
    std::cin >> k;

    std::vector<std::string> noms(static_cast<size_t>(k));
    std::vector<long long> composantes(static_cast<size_t>(k));
    long long total = 0;
    for (size_t i = 0; i < noms.size(); i++) {
        std::cin >> noms[i] >> composantes[i];
        total += composantes[i];
    }

    long long v = 0;
    std::cin >> v;

    long long pas = total * 4;

    long long decalage = 0;
    for (size_t i = 0; i < noms.size(); i++) {
        std::cout << "ATTRIB " << i << ' ' << noms[i] << " COMPOSANTES " << composantes[i]
                  << " DECALAGE " << decalage << '\n';
        decalage += composantes[i] * 4;
    }

    std::cout << "PAS " << pas << '\n';
    std::cout << "TAMPON " << v * pas << '\n';
    return 0;
}
