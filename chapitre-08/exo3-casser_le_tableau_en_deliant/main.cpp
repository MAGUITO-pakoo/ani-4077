// Chapitre 8, exercice 3 : casser le tableau en deliant.
// MONRHI_GLAD=0 retire la verification de glad (gladUninstallGLDebug),
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

	const unsigned short kCarreIndex[6] = { 0, 1, 2,   0, 2, 3 };

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkInitialise();

	// La minuterie fine : sans elle, un sommeil de 16 ms dure presque 30 ms sous Windows.
	NkChrono::BeginPreciseTiming();

	NkWindowConfig cfg;
	cfg.title = "Chapitre 8 - casser le tableau";
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

	// Avec ou sans la verification de glad (qui lit glGetError apres chaque appel).
	const char *choixGlad = nkentseu::env::GetEnvVar("MONRHI_GLAD");
	const bool avecGlad = !(choixGlad != nullptr && choixGlad[0] == '0');
	if (!avecGlad)
		gladUninstallGLDebug();

	monrhi::Nuanceur nuanceur;
	if (!nuanceur.Compiler(kSommets, kFragments)) {
		logger.Errorf("NUANCEUR REFUSE :\n%s", nuanceur.Erreur().CStr());
		contexte.Detruire();
		NkClose();
		return 1;
	}
	logger.Infof("nuanceur: programme %u, compile et lie", nuanceur.Id());

	monrhi::Maillage carre;
	if (!carre.CreerIndexe(kCarreCoins, 4, kCarreIndex, 6)) {
		logger.Error("le maillage n'a pas pu etre cree");
		contexte.Detruire();
		NkClose();
		return 1;
	}

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

		// On vide la file d'erreurs avant le dessin : une erreur plus ancienne
		// serait lue a la place de la sienne.
		while (glGetError() != GL_NO_ERROR) {}

		// Le carre indexe, seul, au centre de la fenetre.
		nuanceur.Activer();
		carre.Dessiner();

		// Juste apres le dessin.
		const GLenum erreur = glGetError();

		// A la troisieme image, AVANT Echanger : apres, ce ne serait plus
		// l'image qu'on vient de dessiner.
		if (images == 2 && !pixelLu) {
			unsigned char px[4] = { 0, 0, 0, 0 };
			glReadPixels((GLint)(taille.x / 2), (GLint)(taille.y / 2), 1, 1,
						 GL_RGBA, GL_UNSIGNED_BYTE, px);
			logger.Infof("%s glad : glGetError = 0x%04X, centre (%d, %d, %d)",
						 avecGlad ? "avec" : "sans", (unsigned)erreur,
						 (int)px[0], (int)px[1], (int)px[2]);
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