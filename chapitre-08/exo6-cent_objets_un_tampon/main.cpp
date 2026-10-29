// Chapitre 8, exercice 6 : cent objets, un tampon.
// MONRHI_CENT=1 choisit la version a cent tampons (sinon : un seul tampon).
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

using namespace nkentseu;

namespace {

	const float kPetroleR = 0.05f;
	const float kPetroleV = 0.25f;
	const float kPetroleB = 0.28f;

	// Le nuanceur de sommets apprend deux uniformes : un decalage et une teinte.
	const char *kSommets = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aCouleur;
uniform vec2 uDecalage;
uniform vec3 uTeinte;
out vec3 vCouleur;
void main() {
    vCouleur = aCouleur * uTeinte;
    gl_Position = vec4(aPosition.xy + uDecalage, aPosition.z, 1.0);
}
)";

	const char *kFragments = R"(
in vec3 vCouleur;

out vec4 oCouleur;

void main() {
    oCouleur = vec4(vCouleur, 1.0);
}
)";

	// Dix rangees de dix carres.
	const int kColonnes = 10;
	const int kRangees = 10;
	const int kObjets = kColonnes * kRangees;
	const float kPas = 0.18f;
	const float kDemi = 0.07f;

	// Cinq cents images mesurees, les trente premieres ecartees.
	const unsigned long long kEcartees = 30;
	const unsigned long long kMesurees = 500;

	// Un carre centre sur l'origine : quatre coins et six index.
	const monrhi::Sommet kCoinsBase[4] = {
		{ -kDemi, -kDemi, 0.0f,  0.96f, 0.62f, 0.14f },
		{  kDemi, -kDemi, 0.0f,  0.20f, 0.75f, 0.70f },
		{  kDemi,  kDemi, 0.0f,  0.90f, 0.30f, 0.35f },
		{ -kDemi,  kDemi, 0.0f,  0.95f, 0.95f, 0.95f },
	};
	const unsigned short kIndex[6] = { 0, 1, 2,   0, 2, 3 };

	double MaintenantNs() {
		return (double)NkChrono::Now().nanoseconds;
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkInitialise();
	NkChrono::BeginPreciseTiming();

	NkWindowConfig cfg;
	cfg.title = "Chapitre 8 - cent objets, un tampon";
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
	// glad verifie chaque appel d'OpenGL : on retire cette verification pour mesurer.
	gladUninstallGLDebug();
	logger.Infof("contexte : %s", contexte.Decrire().CStr());

	monrhi::Nuanceur nuanceur;
	if (!nuanceur.Compiler(kSommets, kFragments)) {
		logger.Errorf("NUANCEUR REFUSE :\n%s", nuanceur.Erreur().CStr());
		contexte.Detruire();
		NkClose();
		return 1;
	}
	logger.Infof("nuanceur: programme %u, compile et lie", nuanceur.Id());

	// La version : lue dans l'environnement.
	const char *choix = nkentseu::env::GetEnvVar("MONRHI_CENT");
	const bool centTampons = choix != nullptr && choix[0] == '1';

	// Pour chaque objet : son decalage et sa teinte, les memes dans les deux versions.
	float decX[kObjets], decY[kObjets];
	float teinteR[kObjets], teinteV[kObjets], teinteB[kObjets];
	for (int j = 0; j < kRangees; ++j) {
		for (int i = 0; i < kColonnes; ++i) {
			const int k = j * kColonnes + i;
			decX[k] = ((float)i - 4.5f) * kPas;
			decY[k] = ((float)j - 4.5f) * kPas;
			teinteR[k] = 0.35f + 0.65f * ((float)i / 9.0f);
			teinteV[k] = 0.35f + 0.65f * ((float)j / 9.0f);
			teinteB[k] = 0.80f;
		}
	}

	// Les maillages. Les octets sont comptes sur ce qu'on cree, avec sizeof.
	monrhi::Maillage unique;
	monrhi::Maillage cent[kObjets];
	size_t octets = 0;
	bool ok = true;

	if (!centTampons) {
		ok = unique.CreerIndexe(kCoinsBase, 4, kIndex, 6);
		octets = sizeof(kCoinsBase) + sizeof(kIndex);
	} else {
		for (int k = 0; k < kObjets && ok; ++k) {
			// Chaque maillage recoit ses quatre coins deja decales et teintes.
			monrhi::Sommet coins[4];
			for (int c = 0; c < 4; ++c) {
				coins[c] = kCoinsBase[c];
				coins[c].x = kCoinsBase[c].x + decX[k];
				coins[c].y = kCoinsBase[c].y + decY[k];
				coins[c].r = kCoinsBase[c].r * teinteR[k];
				coins[c].v = kCoinsBase[c].v * teinteV[k];
				coins[c].b = kCoinsBase[c].b * teinteB[k];
			}
			ok = cent[k].CreerIndexe(coins, 4, kIndex, 6);
			octets += sizeof(coins) + sizeof(kIndex);
		}
	}
	if (!ok) {
		logger.Error("un maillage n'a pas pu etre cree");
		contexte.Detruire();
		NkClose();
		return 1;
	}

	// MONRHI_VOIR=1 : apres les mesures, la fenetre reste ouverte pour regarder la grille.
	const char *choixVoir = nkentseu::env::GetEnvVar("MONRHI_VOIR");
	const bool voir = choixVoir != nullptr && choixVoir[0] == '1';

	unsigned long long images = 0;
	double debutPrecedent = 0.0;
	double sommeImage = 0.0;
	double sommeDessin = 0.0;
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

		// Le debut de l'image : l'intervalle entre deux debuts est le temps d'une image.
		const double debut = MaintenantNs();

		const auto taille = fenetre.GetSize();
		glViewport(0, 0, (GLsizei)taille.x, (GLsizei)taille.y);
		glClearColor(kPetroleR, kPetroleV, kPetroleB, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Le programme est active une seule fois par image.
		nuanceur.Activer();

		// Le temps de dessin : la boucle qui pose les uniformes et dessine.
		const double avantDessin = MaintenantNs();
		if (!centTampons) {
			for (int k = 0; k < kObjets; ++k) {
				nuanceur.PoserVec2("uDecalage", decX[k], decY[k]);
				nuanceur.PoserVec3("uTeinte", teinteR[k], teinteV[k], teinteB[k]);
				unique.Dessiner();
			}
		} else {
			// Uniformes neutres : une teinte jamais posee vaudrait zero, donc du noir.
			nuanceur.PoserVec2("uDecalage", 0.0f, 0.0f);
			nuanceur.PoserVec3("uTeinte", 1.0f, 1.0f, 1.0f);
			for (int k = 0; k < kObjets; ++k)
				cent[k].Dessiner();
		}
		const double apresDessin = MaintenantNs();

		contexte.Echanger();
		NkChrono::YieldThread();

		// On ecarte les trente premieres images, puis on moyenne sur cinq cents.
		if (images >= kEcartees && images < kEcartees + kMesurees) {
			sommeImage += debut - debutPrecedent;
			sommeDessin += apresDessin - avantDessin;
		}
		debutPrecedent = debut;

		++images;

		// A la fin des cinq cents images mesurees : on imprime la ligne, une seule fois.
		if (images == kEcartees + kMesurees) {
			const double moyenneImage = sommeImage / (double)kMesurees / 1.0e6;
			const double moyenneDessin = sommeDessin / (double)kMesurees / 1.0e6;
			logger.Infof("%s : %d objets, %.3f ms par image, dessin %.3f ms, %zu octets",
						 centTampons ? "cent tampons" : "un tampon", kObjets,
						 moyenneImage, moyenneDessin, octets);
			if (!voir)
				break;
		}
	}

	logger.Infof("%llu images, fermeture propre", images);

	// L'ordre inverse de la creation.
	for (int k = kObjets - 1; k >= 0; --k)
		cent[k].Detruire();
	unique.Detruire();
	nuanceur.Detruire();
	contexte.Detruire();
	NkChrono::EndPreciseTiming();
	NkClose();
	return 0;
}