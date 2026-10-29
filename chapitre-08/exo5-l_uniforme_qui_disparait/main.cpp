// Chapitre 8, exercice 5 : l'uniforme qui disparait.
// Trois versions du meme nuanceur de sommets. Seule la ligne qui calcule
// vCouleur change : uInutile n'y est pas employe, puis employe avec un
// coefficient nul, puis avec un coefficient de 0.001.
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
#include "MonRhi/Nuanceur.h"
#include "MonRhi/Gl.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>

using namespace nkentseu;

namespace {

	// Version 1 : uInutile est declare et jamais employe.
	const char *kSommetsJamais = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aCouleur;
uniform vec2 uDecalage;
uniform vec3 uTeinte;
uniform vec3 uInutile;
out vec3 vCouleur;
void main() {
    vCouleur = aCouleur * uTeinte;
    gl_Position = vec4(aPosition.xy + uDecalage, aPosition.z, 1.0);
}
)";

	// Version 2 : uInutile entre dans le calcul, avec un coefficient nul.
	const char *kSommetsNul = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aCouleur;
uniform vec2 uDecalage;
uniform vec3 uTeinte;
uniform vec3 uInutile;
out vec3 vCouleur;
void main() {
    vCouleur = aCouleur * uTeinte + 0.0 * uInutile;
    gl_Position = vec4(aPosition.xy + uDecalage, aPosition.z, 1.0);
}
)";

	// Version 3 : le meme calcul, avec un coefficient de 0.001.
	const char *kSommetsMilli = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aCouleur;
uniform vec2 uDecalage;
uniform vec3 uTeinte;
uniform vec3 uInutile;
out vec3 vCouleur;
void main() {
    vCouleur = aCouleur * uTeinte + 0.001 * uInutile;
    gl_Position = vec4(aPosition.xy + uDecalage, aPosition.z, 1.0);
}
)";

	// Le nuanceur de fragments ne change jamais.
	const char *kFragments = R"(
in vec3 vCouleur;

out vec4 oCouleur;

void main() {
    oCouleur = vec4(vCouleur, 1.0);
}
)";

	// Compile une version, puis lit les trois emplacements APRES la liaison.
	// Un emplacement est un numero choisi par le pilote : -1 veut dire que
	// l'uniforme n'existe plus dans le programme lie.
	bool Releve(const char *titre, const char *sommets) {
		monrhi::Nuanceur nuanceur;
		if (!nuanceur.Compiler(sommets, kFragments)) {
			logger.Errorf("NUANCEUR REFUSE (%s) :\n%s", titre, nuanceur.Erreur().CStr());
			return false;
		}
		const int decalage = (int)glGetUniformLocation(nuanceur.Id(), "uDecalage");
		const int teinte = (int)glGetUniformLocation(nuanceur.Id(), "uTeinte");
		const int inutile = (int)glGetUniformLocation(nuanceur.Id(), "uInutile");
		logger.Infof("%s : uDecalage %d, uTeinte %d, uInutile %d", titre, decalage, teinte, inutile);
		nuanceur.Detruire();
		return true;
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkInitialise();
	NkChrono::BeginPreciseTiming();

	NkWindowConfig cfg;
	cfg.title = "Chapitre 8 - l'uniforme qui disparait";
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

	// Les trois lignes, une par version, dans cet ordre.
	const bool ok = Releve("jamais employe", kSommetsJamais) &&
					Releve("coefficient nul", kSommetsNul) &&
					Releve("coefficient 0.001", kSommetsMilli);
	if (!ok)
		logger.Error("une version n'a pas pu etre compilee");

	logger.Infof("fermeture propre");

	contexte.Detruire();
	NkChrono::EndPreciseTiming();
	NkClose();
	return ok ? 0 : 1;
}