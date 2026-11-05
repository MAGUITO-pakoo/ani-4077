#include <iostream>
#include <string>
#include <vector>

// Division entière arrondie vers le bas, y compris pour un nombre négatif.
static long long arrondiBas(long long a, long long b) {
    long long q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) {
        q--;
    }
    return q;
}

static long long resteTexture(long long a, long long m) {
    return ((a % m) + m) % m;
}

int main() {
    long long largeur = 0, hauteur = 0, chargeur = 0, nuanceur = 0, nb = 0;
    std::string mode;
    std::cin >> largeur >> hauteur >> chargeur >> nuanceur >> mode >> nb;

    std::vector<std::string> image(static_cast<size_t>(hauteur));
    for (auto& ligne : image) {
        std::cin >> ligne;
    }

    const bool repete = (mode == "GL_REPEAT");

    for (long long q = 0; q < nb; q++) {
        long long u = 0, v = 0;
        std::cin >> u >> v;

        long long vv = nuanceur == 1 ? 1000 - v : v;
        long long colonne = arrondiBas(u * largeur, 1000);
        long long ligne = arrondiBas(vv * hauteur, 1000);

        if (repete) {
            colonne = resteTexture(colonne, largeur);
            ligne = resteTexture(ligne, hauteur);
        } else {
            if (colonne < 0) colonne = 0;
            if (colonne > largeur - 1) colonne = largeur - 1;
            if (ligne < 0) ligne = 0;
            if (ligne > hauteur - 1) ligne = hauteur - 1;
        }

        // La ligne 0 d'OpenGL est la première ligne reçue.
        long long ligneFichier = chargeur == 1 ? hauteur - 1 - ligne : ligne;
        char c = image[static_cast<size_t>(ligneFichier)][static_cast<size_t>(colonne)];
        std::cout << u << ' ' << v << ' ' << c << '\n';
    }

    long long corrections = chargeur + nuanceur;
    std::cout << "CORRECTIONS " << corrections << '\n';
    std::cout << (corrections == 1 ? "IMAGE DROITE" : "IMAGE RETOURNEE") << '\n';
    return 0;
}
