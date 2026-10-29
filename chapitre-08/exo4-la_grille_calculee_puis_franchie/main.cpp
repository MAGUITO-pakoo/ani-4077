// Chapitre 8, exercice 4 : la grille, calculee puis franchie.
// MONRHI_INDEX=4 dessine la grande grille avec des index sur quatre octets (sinon deux).
// MONRHI_IMAGES=n arrete au bout de n images.
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKWindow/Core/NkSurface.h"
#include "NKEvent/NkEventSystem.h"
#include "NKEvent/NkWindowEvent.h"

#include "NKLogger/NkLog.h"
#include "NKPlatform/NkEnv.h"
#include "NKTime/NkChrono.h"
#include "MonRhi/MonRhi.h"
#include "MonRhi/Contexte.h"
#include "MonRhi/Maillage.h"
#include "MonRhi/Nuanceur.h"
#include "MonRhi/Gl.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace nkentseu;

namespace {

	const float kPetroleR = 0.05f;
	const float kPetroleV = 0.25f;
	const float kPetroleB = 0.28f;

	const char *kSommets = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aCouleur;

out vec3 vCouleur;

void main() {
    vCouleur = aCouleur;
    gl_Position = vec4(aPosition, 1.0);
}
)";

	const char *kFragments = R"(
in vec3 vCouleur;

out vec4 oCouleur;

void main() {
    oCouleur = vec4(vCouleur, 1.0);
}
)";

	// Une grille de n x n sommets sur le carre [-0.9, 0.9], deux triangles par case.
	//   coins : les n*n sommets, chacun ecrit une fois
	//   index : 6 index par case, qui disent dans quel ordre les lire
	//   sans  : la meme grille SANS index, chaque triangle recopie ses trois sommets
	// Les rangees sont coloriees une sur deux, d'une couleur franche.
	void FabriquerGrille(int n, std::vector<monrhi::Sommet> &coins, std::vector<unsigned int> &index,
						 std::vector<monrhi::Sommet> &sans) {
		coins.clear();
		index.clear();
		sans.clear();
		coins.reserve((size_t)n * (size_t)n);

		for (int j = 0; j < n; ++j) {
			for (int i = 0; i < n; ++i) {
				const float x = -0.9f + 1.8f * (float)i / (float)(n - 1);
				const float y = -0.9f + 1.8f * (float)j / (float)(n - 1);
				const bool paire = (j % 2) == 0;
				monrhi::Sommet s = { x, y, 0.0f,
									 paire ? 0.95f : 0.10f, paire ? 0.35f : 0.65f, paire ? 0.15f : 0.75f };
				coins.push_back(s);
			}
		}

		for (int j = 0; j < n - 1; ++j) {
			for (int i = 0; i < n - 1; ++i) {
				const unsigned int a = (unsigned int)(j * n + i);
				const unsigned int b = a + 1;
				const unsigned int c = a + (unsigned int)n;
				const unsigned int d = c + 1;
				const unsigned int tri[6] = { a, b, d,   a, d, c };
				for (int k = 0; k < 6; ++k) {
					index.push_back(tri[k]);
					sans.push_back(coins[tri[k]]);
				}
			}
		}
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkInitialise();
	NkChrono::BeginPreciseTiming();

	NkWindowConfig cfg;
	cfg.title = "Chapitre 8 - la grille, calculee puis franchie";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;

	NkWindow fenetre(cfg);
	if (!fenetre.IsValid()) {
		logger.Error("la fenetre ne s'est pas ouverte");
		NkClose();
		return 1;
	}

	const NkSurfaceDesc surface = fenetre.GetSurfaceDesc();
	logger.Infof("%s", monrhi::DecrireSurface(surface).CStr());

	monrhi::Contexte contexte;
	if (!contexte.Creer(surface)) {
		logger.Error("contexte impossible : on s'arrete ici.");
		NkClose();
		return 1;
	}
	logger.Infof("contexte : %s", contexte.Decrire().CStr());

	monrhi::Nuanceur nuanceur;
	if (!nuanceur.Compiler(kSommets, kFragments)) {
		logger.Errorf("NUANCEUR REFUSE :\n%s", nuanceur.Erreur().CStr());
		contexte.Detruire();
		NkClose();
		return 1;
	}
	logger.Infof("nuanceur: programme %u, compile et lie", nuanceur.Id());

	std::vector<monrhi::Sommet> coins, sans;
	std::vector<unsigned int> index;

	// ── Les trois petites grilles : N = 2, 10, 100 ───────────────────────────
	const int tailles[3] = { 2, 10, 100 };
	for (int t = 0; t < 3; ++t) {
		const int n = tailles[t];
		FabriquerGrille(n, coins, index, sans);

		std::vector<unsigned short> index16(index.size());
		for (size_t k = 0; k < index.size(); ++k)
			index16[k] = (unsigned short)index[k];

		// Les deux maillages existent vraiment : on les cree, puis on les libere.
		monrhi::Maillage mSans, mAvec;
		if (!mSans.Creer(sans.data(), (int)sans.size()) ||
			!mAvec.CreerIndexe(coins.data(), (int)coins.size(), index16.data(), (int)index16.size())) {
			logger.Error("un maillage n'a pas pu etre cree");
			contexte.Detruire();
			NkClose();
			return 1;
		}

		// Les octets sont comptes avec sizeof sur ce qu'on envoie.
		const size_t octetsSans = sans.size() * sizeof(monrhi::Sommet);
		const size_t octetsAvec = coins.size() * sizeof(monrhi::Sommet) + index16.size() * sizeof(unsigned short);
		logger.Infof("N = %d : sans index %zu octets, avec index %zu octets, rapport %.3f", n,
					 octetsSans, octetsAvec, (double)octetsSans / (double)octetsAvec);

		mAvec.Detruire();
		mSans.Detruire();
	}

	// ── La grande grille : 265 x 265 = 70 225 sommets, plus que 65 535 ──────
	const int kGrand = 265;
	FabriquerGrille(kGrand, coins, index, sans);

	// Index sur deux octets : un unsigned short ne sait pas compter au-dela de 65 535.
	std::vector<unsigned short> index16(index.size());
	for (size_t k = 0; k < index.size(); ++k)
		index16[k] = (unsigned short)index[k];

	monrhi::Maillage grand16, grand32;
	if (!grand16.CreerIndexe(coins.data(), (int)coins.size(), index16.data(), (int)index16.size()) ||
		!grand32.CreerIndexe32(coins.data(), (int)coins.size(), index.data(), (int)index.size())) {
		logger.Error("la grande grille n'a pas pu etre creee");
		contexte.Detruire();
		NkClose();
		return 1;
	}

	const size_t octets2 = coins.size() * sizeof(monrhi::Sommet) + index16.size() * sizeof(unsigned short);
	const size_t octets4 = coins.size() * sizeof(monrhi::Sommet) + index.size() * sizeof(unsigned int);
	logger.Infof("grille %d : %d sommets, index sur 2 octets, %zu octets", kGrand, kGrand * kGrand, octets2);
	logger.Infof("grille %d : %d sommets, index sur 4 octets, %zu octets", kGrand, kGrand * kGrand, octets4);

	const char *choixIndex = nkentseu::env::GetEnvVar("MONRHI_INDEX");
	const bool quatreOctets = choixIndex != nullptr && choixIndex[0] == '4';
	const monrhi::Maillage &choisi = quatreOctets ? grand32 : grand16;
	logger.Infof("on dessine la grille avec des index sur %d octets", quatreOctets ? 4 : 2);

	const char *limite = nkentseu::env::GetEnvVar("MONRHI_IMAGES");
	const unsigned long long maxImages = limite != nullptr ? std::strtoull(limite, nullptr, 10) : 0;

	unsigned long long images = 0;
	bool tourne = true;

	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr)
				tourne = false;

			// La surface qui va et vient sur mobile : les quatre evenements.
			if (ev->As<NkWindowSurfaceDestroyedEvent>() != nullptr || ev->As<NkWindowHiddenEvent>() != nullptr)
				contexte.DetacherSurface();
			if (ev->As<NkWindowSurfaceCreatedEvent>() != nullptr || ev->As<NkWindowShownEvent>() != nullptr)
				contexte.RattacherSurface(fenetre.GetSurfaceDesc());
		}
		if (!tourne)
			break;

		if (!contexte.PeutDessiner()) {
			NkChrono::Sleep((int64)16);
			continue;
		}

		const auto taille = fenetre.GetSize();
		glViewport(0, 0, (GLsizei)taille.x, (GLsizei)taille.y);

		glClearColor(kPetroleR, kPetroleV, kPetroleB, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// La grille dans un carre centre de la fenetre : la meme pour les deux captures.
		const int c = (int)(taille.x < taille.y ? taille.x : taille.y);
		glViewport(((int)taille.x - c) / 2, ((int)taille.y - c) / 2, c, c);

		nuanceur.Activer();
		choisi.Dessiner();

		contexte.Echanger();
		NkChrono::YieldThread();

		++images;
		if (maxImages != 0 && images >= maxImages)
			break;
	}

	logger.Infof("%llu images, fermeture propre", images);

	grand32.Detruire();
	grand16.Detruire();
	nuanceur.Detruire();
	contexte.Detruire();
	NkChrono::EndPreciseTiming();
	NkClose();
	return 0;
}