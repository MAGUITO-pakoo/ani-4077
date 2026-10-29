// Chapitre 8, exercice 1 : le carre des deux facons (sans index, avec index).
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

	// ── LE NUANCEUR DE SOMMETS ───────────────────────────────────────────────
	// Il s'exécute UNE FOIS PAR SOMMET, donc trois fois pour un triangle.
	//
	// Remarquez ce qui n'y est pas : aucune ligne « #version ». Elle est posée
	// par Nuanceur::EnteteGlsl selon le système, et c'est ce qui permet à ces
	// quinze lignes d'être les mêmes sur Windows, Linux, Android, HarmonyOS et
	// le Web.
	//
	// gl_Position est la SEULE sortie obligatoire. Elle a quatre composantes,
	// pas trois : la quatrième, w, vaut 1 pour l'instant et deviendra le
	// sujet du chapitre 16, quand la perspective arrivera.
	const char *kSommets = R"(
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aCouleur;

out vec3 vCouleur;

void main() {
    vCouleur = aCouleur;
    gl_Position = vec4(aPosition, 1.0);
}
)";

	// ── LE NUANCEUR DE FRAGMENTS ─────────────────────────────────────────────
	// Il s'exécute UNE FOIS PAR PIXEL COUVERT. Pour un triangle qui occupe la
	// moitié d'une fenêtre 1280x720, cela fait plus de 460 000 exécutions par
	// image, à soixante images par seconde. C'est pour cela qu'on ne met pas
	// de « if » compliqués ici.
	//
	// vCouleur n'est PAS la couleur d'un sommet : c'est la couleur INTERPOLÉE
	// entre les trois, calculée par la carte pour ce pixel précis. On n'a
	// donné que trois couleurs, on en obtient des centaines de milliers.
	const char *kFragments = R"(
in vec3 vCouleur;

out vec4 oCouleur;

void main() {
    oCouleur = vec4(vCouleur, 1.0);
}
)";

	// Les deux carres : le premier sans index (6 sommets), le second avec
	// quatre coins et six index.
	const monrhi::Sommet kCarreSansIndex[6] = {
		{ -0.20f, -0.20f, 0.0f,  0.96f, 0.62f, 0.14f },
		{  0.20f, -0.20f, 0.0f,  0.20f, 0.75f, 0.70f },
		{  0.20f,  0.20f, 0.0f,  0.90f, 0.30f, 0.35f },
		{ -0.20f, -0.20f, 0.0f,  0.96f, 0.62f, 0.14f },   // repris
		{  0.20f,  0.20f, 0.0f,  0.90f, 0.30f, 0.35f },   // repris
		{ -0.20f,  0.20f, 0.0f,  0.95f, 0.95f, 0.95f },
	};

	const monrhi::Sommet kCarreCoins[4] = {
		{ -0.20f, -0.20f, 0.0f,  0.96f, 0.62f, 0.14f },
		{  0.20f, -0.20f, 0.0f,  0.20f, 0.75f, 0.70f },
		{  0.20f,  0.20f, 0.0f,  0.90f, 0.30f, 0.35f },
		{ -0.20f,  0.20f, 0.0f,  0.95f, 0.95f, 0.95f },
	};
	const unsigned short kCarreIndex[6] = { 0, 1, 2,   0, 2, 3 };

	// La maison : le carre avec un cinquieme coin, le toit, au-dessus.
	const monrhi::Sommet kMaisonSansIndex[9] = {
		{ -0.20f, -0.20f, 0.0f,  0.96f, 0.62f, 0.14f },
		{  0.20f, -0.20f, 0.0f,  0.20f, 0.75f, 0.70f },
		{  0.20f,  0.20f, 0.0f,  0.90f, 0.30f, 0.35f },
		{ -0.20f, -0.20f, 0.0f,  0.96f, 0.62f, 0.14f },   // repris
		{  0.20f,  0.20f, 0.0f,  0.90f, 0.30f, 0.35f },   // repris
		{ -0.20f,  0.20f, 0.0f,  0.95f, 0.95f, 0.95f },
		{ -0.20f,  0.20f, 0.0f,  0.95f, 0.95f, 0.95f },   // repris
		{  0.20f,  0.20f, 0.0f,  0.90f, 0.30f, 0.35f },   // repris
		{  0.00f,  0.40f, 0.0f,  0.60f, 0.20f, 0.15f },   // le toit
	};

	const monrhi::Sommet kMaisonCoins[5] = {
		{ -0.20f, -0.20f, 0.0f,  0.96f, 0.62f, 0.14f },
		{  0.20f, -0.20f, 0.0f,  0.20f, 0.75f, 0.70f },
		{  0.20f,  0.20f, 0.0f,  0.90f, 0.30f, 0.35f },
		{ -0.20f,  0.20f, 0.0f,  0.95f, 0.95f, 0.95f },
		{  0.00f,  0.40f, 0.0f,  0.60f, 0.20f, 0.15f },
	};
	const unsigned short kMaisonIndex[9] = { 0, 1, 2,   0, 2, 3,   3, 2, 4 };

	// Imprime les trois lignes d'une forme. Les octets sont comptes avec sizeof.
	void Rapport(const char *nom, int sommetsSans, size_t octetsSans,
				 int nombreIndex, int sommetsAvec, size_t octetsAvec) {
		logger.Infof("%s SANS index : %d sommets, %zu octets", nom, sommetsSans, octetsSans);
		logger.Infof("%s AVEC index : %d index sur %d sommets, %zu octets", nom, nombreIndex,
					 sommetsAvec, octetsAvec);
		const size_t eco = octetsSans - octetsAvec;
		logger.Infof("economie : %zu octets, soit %.1f %%", eco,
					 100.0 * (double)eco / (double)octetsSans);
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkInitialise();

	// LA MINUTERIE FINE, ET CE N'EST PAS UN DETAIL.
	// Sans cet appel, un sommeil de 16 ms dure 29,76 ms sur Windows :
	// mesure du moteur, 15 aout 2026, 40 repetitions. La boucle ne tourne
	// alors pas a 60 images par seconde mais a 33, et rien ne le dit.
	NkChrono::BeginPreciseTiming();

	NkWindowConfig cfg;
	cfg.title = "Chapitre 8 - le carre des deux facons";
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

	// ── CE QUE L'ÉTAPE 04 NE FAISAIT PAS ─────────────────────────────────────
	// L'en-tête GLSL réellement posé s'imprime : c'est la preuve visible que
	// les mêmes quinze lignes de source produisent deux GLSL différents.
	logger.Infof("glsl    : %s",
				monrhi::Nuanceur::EnteteGlsl(false).CStr());

	monrhi::Nuanceur nuanceur;
	if (!nuanceur.Compiler(kSommets, kFragments)) {
		// ON IMPRIME LE JOURNAL. Sans ces trois lignes, un nuanceur qui ne
		// compile pas donne un ecran vide et un programme qui a l'air d'aller
		// tres bien. Le message du pilote existe, il faut le demander.
		logger.Errorf("NUANCEUR REFUSE :\n%s", nuanceur.Erreur().CStr());
		contexte.Detruire();
		NkClose();
		return 1;
	}
	logger.Infof("nuanceur: programme %u, compile et lie", nuanceur.Id());

	// Les chiffres : un sommet = sizeof(Sommet), un index = sizeof(unsigned short).
	Rapport("carre", 6, sizeof(kCarreSansIndex), 6, 4,
			sizeof(kCarreCoins) + sizeof(kCarreIndex));
	Rapport("maison", 9, sizeof(kMaisonSansIndex), 9, 5,
			sizeof(kMaisonCoins) + sizeof(kMaisonIndex));

	monrhi::Maillage carreSans, carreAvec, maisonSans, maisonAvec;
	if (!carreSans.Creer(kCarreSansIndex, 6) ||
		!carreAvec.CreerIndexe(kCarreCoins, 4, kCarreIndex, 6) ||
		!maisonSans.Creer(kMaisonSansIndex, 9) ||
		!maisonAvec.CreerIndexe(kMaisonCoins, 5, kMaisonIndex, 9)) {
		logger.Error("un maillage n'a pas pu etre cree");
		contexte.Detruire();
		NkClose();
		return 1;
	}

	// Meme l'environnement passe par le moteur : NkEnv enveloppe
	// GetEnvironmentVariable sous Windows et getenv ailleurs.
	const char *limite = nkentseu::env::GetEnvVar("MONRHI_IMAGES");
	const unsigned long long maxImages = limite != nullptr ? std::strtoull(limite, nullptr, 10) : 0;

	unsigned long long images = 0;
	bool tourne = true;

	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr)
				tourne = false;

			// La surface qui va et vient sur mobile, comme au chapitre 5. On
			// écoute les QUATRE événements : Android et HarmonyOS ne disent
			// pas la même chose avec les mêmes mots.
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

		// Quatre cases carrees. En haut le carre, en bas la maison.
		// A gauche sans index, a droite avec index : on doit voir la meme image.
		const int c = (int)(taille.y / 2 < taille.x / 2 ? taille.y / 2 : taille.x / 2);
		const int x0 = ((int)taille.x - 2 * c) / 2;
		const int y0 = ((int)taille.y - 2 * c) / 2;

		nuanceur.Activer();
		glViewport(x0,     y0 + c, c, c);  carreSans.Dessiner();
		glViewport(x0 + c, y0 + c, c, c);  carreAvec.Dessiner();
		glViewport(x0,     y0,     c, c);  maisonSans.Dessiner();
		glViewport(x0 + c, y0,     c, c);  maisonAvec.Dessiner();

		contexte.Echanger();

		// ON REND LA MAIN. Sur le Web, cette ligne est la difference entre
		// une demo qui tourne et un onglet gele : YieldThread appelle
		// emscripten_sleep(0), qui laisse le navigateur dessiner et
		// reprendre ensuite ici meme. Ailleurs, elle cede le quantum au
		// systeme et ne coute rien.
		NkChrono::YieldThread();

		++images;
		if (maxImages != 0 && images >= maxImages)
			break;
	}

	logger.Infof("%llu images, fermeture propre", images);

	// L'ordre inverse de la création, toujours. Le maillage et le nuanceur
	// vivent DANS le contexte : ils partent avant lui.
	maisonAvec.Detruire();
	maisonSans.Detruire();
	carreAvec.Detruire();
	carreSans.Detruire();
	nuanceur.Detruire();
	contexte.Detruire();
	NkChrono::EndPreciseTiming();
	NkClose();
	return 0;
}