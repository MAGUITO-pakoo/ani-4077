// Chapitre 8, exercice 2 : l'ordre des index.
// MONRHI_ORDRE=1|2|3 choisit l'ordre des index, MONRHI_TRI=1 active le tri des faces,
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

using namespace nkentseu;

namespace {

	const float kPetroleR = 0.05f;
	const float kPetroleV = 0.25f;
	const float kPetroleB = 0.28f;

	// Le nuanceur de sommets : une fois par sommet. Pas de ligne #version,
	// elle est posee par Nuanceur::EnteteGlsl selon le systeme.
	const char *kSommets = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aCouleur;

out vec3 vCouleur;

void main() {
    vCouleur = aCouleur;
    gl_Position = vec4(aPosition, 1.0);
}
)";

	// Le nuanceur de fragments : une fois par pixel couvert.
	const char *kFragments = R"(
in vec3 vCouleur;

out vec4 oCouleur;

void main() {
    oCouleur = vec4(vCouleur, 1.0);
}
)";

	// Le carre a quatre coins.
	const monrhi::Sommet kCarreCoins[4] = {
		{ -0.20f, -0.20f, 0.0f,  0.96f, 0.62f, 0.14f },
		{  0.20f, -0.20f, 0.0f,  0.20f, 0.75f, 0.70f },
		{  0.20f,  0.20f, 0.0f,  0.90f, 0.30f, 0.35f },
		{ -0.20f,  0.20f, 0.0f,  0.95f, 0.95f, 0.95f },
	};

	// Les trois ordres d'index essayes.
	const unsigned short kIndex1[6] = { 0, 1, 2,   0, 2, 3 };
	const unsigned short kIndex2[6] = { 0, 1, 2,   2, 3, 0 };
	const unsigned short kIndex3[6] = { 0, 2, 1,   0, 3, 2 };

	const unsigned short *kOrdres[3] = { kIndex1, kIndex2, kIndex3 };
	const char *kNomsOrdres[3] = { "{0,1,2, 0,2,3}", "{0,1,2, 2,3,0}", "{0,2,1, 0,3,2}" };

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkInitialise();

	// La minuterie fine : sans elle, un sommeil de 16 ms dure presque 30 ms sous Windows.
	NkChrono::BeginPreciseTiming();

	NkWindowConfig cfg;
	cfg.title = "Chapitre 8 - l'ordre des index";
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

	// Quel ordre, avec ou sans tri : lus dans l'environnement.
	const char *choixOrdre = nkentseu::env::GetEnvVar("MONRHI_ORDRE");
	int ordre = choixOrdre != nullptr ? std::atoi(choixOrdre) : 1;
	if (ordre < 1 || ordre > 3)
		ordre = 1;

	const char *choixTri = nkentseu::env::GetEnvVar("MONRHI_TRI");
	const bool tri = choixTri != nullptr && choixTri[0] == '1';

	monrhi::Maillage carre;
	if (!carre.CreerIndexe(kCarreCoins, 4, kOrdres[ordre - 1], 6)) {
		logger.Error("le maillage n'a pas pu etre cree");
		contexte.Detruire();
		NkClose();
		return 1;
	}

	// Le tri des faces : active UNE fois, avant la boucle, et seulement pour l'essai.
	if (tri)
		glEnable(GL_CULL_FACE);

	const char *limite = nkentseu::env::GetEnvVar("MONRHI_IMAGES");
	const unsigned long long maxImages = limite != nullptr ? std::strtoull(limite, nullptr, 10) : 0;

	unsigned long long images = 0;
	bool pixelLu = false;
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

		// Le carre indexe, seul, au centre de la fenetre.
		nuanceur.Activer();
		carre.Dessiner();

		// A la troisieme image, AVANT Echanger : apres, ce ne serait plus
		// l'image qu'on vient de dessiner. Le centre est la moitie de la taille,
		// soit (640, 360) pour une fenetre de 1280x720.
		if (images == 2 && !pixelLu) {
			unsigned char px[4] = { 0, 0, 0, 0 };
			glReadPixels((GLint)(taille.x / 2), (GLint)(taille.y / 2), 1, 1,
						 GL_RGBA, GL_UNSIGNED_BYTE, px);
			logger.Infof("ordre %s, %s tri : centre (%d, %d, %d)", kNomsOrdres[ordre - 1],
						 tri ? "avec" : "sans", (int)px[0], (int)px[1], (int)px[2]);
			pixelLu = true;
		}

		contexte.Echanger();

		// On rend la main : sur le Web, c'est ce qui evite l'onglet gele.
		NkChrono::YieldThread();

		++images;
		if (maxImages != 0 && images >= maxImages)
			break;
	}

	logger.Infof("%llu images, fermeture propre", images);

	// L'ordre inverse de la creation : le maillage et le nuanceur partent avant le contexte.
	carre.Detruire();
	nuanceur.Detruire();
	contexte.Detruire();
	NkChrono::EndPreciseTiming();
	NkClose();
	return 0;
}