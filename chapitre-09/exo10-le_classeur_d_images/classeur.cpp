#include <iostream>
#include <map>
#include <string>

static bool estLineaire(const std::string& nom) {
    size_t pos = nom.rfind('_');
    if (pos == std::string::npos) {
        return false;
    }
    std::string mot = nom.substr(pos + 1);
    return mot == "normal" || mot == "rugosite" || mot == "hauteur" || mot == "metal" ||
           mot == "occlusion";
}

int main() {
    int n = 0;
    std::cin >> n;

    std::map<std::string, int> references;
    int creations = 0, reutilisations = 0, srgb = 0, lineaires = 0;

    for (int i = 0; i < n; i++) {
        std::string cmd, nom;
        std::cin >> cmd >> nom;

        auto it = references.find(nom);
        if (cmd == "DEMANDE") {
            if (it != references.end()) {
                it->second++;
                reutilisations++;
                std::cout << "REUTILISE " << nom << " REFS " << it->second << '\n';
            } else {
                references[nom] = 1;
                creations++;
                bool lin = estLineaire(nom);
                if (lin) {
                    lineaires++;
                } else {
                    srgb++;
                }
                std::cout << "CREE " << nom << " ESPACE " << (lin ? "LINEAIRE" : "SRGB")
                          << " REFS 1\n";
            }
        } else if (cmd == "RENDS") {
            if (it == references.end()) {
                std::cout << "RENDS " << nom << " INCONNU\n";
            } else {
                it->second--;
                if (it->second == 0) {
                    references.erase(it);
                    std::cout << "DETRUIT " << nom << '\n';
                } else {
                    std::cout << "RENDS " << nom << " REFS " << it->second << '\n';
                }
            }
        }
    }

    std::cout << "CREATIONS " << creations << '\n';
    std::cout << "REUTILISATIONS " << reutilisations << '\n';
    std::cout << "VIVANTES " << references.size() << '\n';
    std::cout << "SRGB " << srgb << '\n';
    std::cout << "LINEAIRES " << lineaires << '\n';
    return 0;
}
