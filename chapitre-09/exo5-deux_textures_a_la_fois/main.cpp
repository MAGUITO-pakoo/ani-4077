#include <iostream>
#include <string>
#include <vector>

int main() {
    int e = 0, n = 0;
    std::cin >> e >> n;

    std::vector<std::string> noms(static_cast<size_t>(e));
    for (auto& nom : noms) {
        std::cin >> nom;
    }

    int courante = 0;
    std::vector<long long> unites(16, 0);
    std::vector<int> echantillonneurs(static_cast<size_t>(e), 0);

    int dessins = 0, justes = 0, faux = 0, ignores = 0;

    for (int i = 0; i < n; i++) {
        std::string cmd;
        std::cin >> cmd;

        if (cmd == "ACTIVE") {
            int u = 0;
            std::cin >> u;
            courante = u;
        } else if (cmd == "BIND") {
            long long t = 0;
            std::cin >> t;
            unites[static_cast<size_t>(courante)] = t;
        } else if (cmd == "UNIFORM1I" || cmd == "UNIFORM1F") {
            std::string nom;
            int u = 0;
            std::cin >> nom >> u;
            if (cmd == "UNIFORM1F") {
                ignores++;
            } else {
                for (size_t k = 0; k < noms.size(); k++) {
                    if (noms[k] == nom) {
                        echantillonneurs[k] = u;
                    }
                }
            }
        } else if (cmd == "DESSINE") {
            dessins++;
            std::string detail;
            for (size_t k = 0; k < noms.size(); k++) {
                long long attendu = 0;
                std::cin >> attendu;
                long long lu = unites[static_cast<size_t>(echantillonneurs[k])];
                if (lu != attendu) {
                    detail += " " + noms[k] + "=" + std::to_string(lu) + "/" +
                              std::to_string(attendu);
                }
            }
            if (detail.empty()) {
                std::cout << "DESSIN " << dessins << " JUSTE\n";
                justes++;
            } else {
                std::cout << "DESSIN " << dessins << " FAUX" << detail << '\n';
                faux++;
            }
        }
    }

    std::cout << "JUSTES " << justes << '\n';
    std::cout << "FAUX " << faux << '\n';
    std::cout << "UNIFORM1F IGNORES " << ignores << '\n';
    return 0;
}
