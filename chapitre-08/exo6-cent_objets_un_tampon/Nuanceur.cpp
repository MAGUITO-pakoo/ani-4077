// =============================================================================
//  Étape 05 — compiler du GLSL, et surtout LIRE CE QUE LE PILOTE RÉPOND.
// =============================================================================
#include "MonRhi/Nuanceur.h"
#include "MonRhi/Gl.h"

#include "NKContainers/Sequential/NkVector.h"

namespace monrhi {

	// ─────────────────────────────────────────────────────────────────────────
	//  LE SEUL ENDROIT DU LIVRE OÙ LE GLSL DIFFÈRE ENTRE BUREAU ET MOBILE
	//
	//  OpenGL 3.3 core parle GLSL 3.30 ; OpenGL ES 3.0 parle GLSL ES 3.00. Les
	//  deux ont la MÊME grammaire pour tout ce que fait ce chapitre : « in »,
	//  « out », « layout(location=) », « void main() ». Deux choses seules
	//  changent, et elles tiennent dans cette fonction.
	//
	//  1. LA LIGNE DE VERSION. « #version 330 core » d'un côté,
	//     « #version 300 es » de l'autre. Elle doit être la PREMIÈRE ligne non
	//     vide du fichier, sans quoi le pilote refuse tout.
	//
	//  2. LA PRÉCISION, sur mobile seulement. GLSL ES n'a AUCUNE précision par
	//     défaut pour les flottants dans le nuanceur de fragments : sans la
	//     déclarer, le nuanceur ne compile pas, avec un message qui parle de
	//     « no precision specified » et que personne ne comprend la première
	//     fois. Sur bureau, cette ligne est ignorée, mais la poser quand même
	//     ferait diverger les deux sources pour rien.
	//
	//  D'où la règle de ce livre : VOS SOURCES N'ÉCRIVENT JAMAIS « #version ».
	//  Vous écrivez le corps, cette fonction pose l'en-tête. Une source, deux
	//  GLSL, cinq plateformes.
	// ─────────────────────────────────────────────────────────────────────────
	nkentseu::NkString Nuanceur::EnteteGlsl(bool fragments) {
#if defined(NKENTSEU_PLATFORM_ANDROID) || defined(NKENTSEU_PLATFORM_HARMONYOS) ||                                       \
	defined(NKENTSEU_PLATFORM_EMSCRIPTEN)
		nkentseu::NkString entete = "#version 300 es\n";
		if (fragments)
			entete += "precision mediump float;\n";
		return entete;
#else
		(void)fragments;
		return "#version 330 core\n";
#endif
	}

	Nuanceur::~Nuanceur() {
		Detruire();
	}

	unsigned int Nuanceur::CompilerUn(unsigned int type, const nkentseu::NkString &source,
									  const char *nomPourErreur) {
		const nkentseu::NkString complet = EnteteGlsl(type == GL_FRAGMENT_SHADER) + source;
		const char *ptr = complet.CStr();

		const GLuint id = glCreateShader((GLenum)type);
		if (id == 0) {
			mErreur = nkentseu::NkString("glCreateShader a rendu 0 pour ") + nomPourErreur
					  + " : il n'y a probablement pas de contexte courant.";
			return 0;
		}

		glShaderSource(id, 1, &ptr, nullptr);
		glCompileShader(id);

		// ── LE CONTRÔLE QUE TOUT LE MONDE OUBLIE ────────────────────────────
		// glCompileShader ne rend RIEN. Un nuanceur qui ne compile pas produit
		// un objet parfaitement valide, simplement vide. On DOIT demander.
		GLint ok = GL_FALSE;
		glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
		if (ok == GL_FALSE) {
			GLint taille = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &taille);
			nkentseu::NkVector<char> journal((size_t)(taille > 1 ? taille : 1), '\0');
			glGetShaderInfoLog(id, taille, nullptr, journal.Data());

			mErreur = nkentseu::NkString("compilation du nuanceur ") + nomPourErreur + " :\n"
					  + journal.Data()
					  + "\n--- source reellement envoyee au pilote ---\n" + complet;
			glDeleteShader(id);
			return 0;
		}
		return id;
	}

	bool Nuanceur::Compiler(const nkentseu::NkString &sommets, const nkentseu::NkString &fragments) {
		Detruire();
		mErreur.Clear();

		const GLuint vs = CompilerUn(GL_VERTEX_SHADER, sommets, "sommets");
		if (vs == 0)
			return false;

		const GLuint fs = CompilerUn(GL_FRAGMENT_SHADER, fragments, "fragments");
		if (fs == 0) {
			// Le nuanceur de sommets avait compilé : il faut le libérer, sinon
			// chaque tentative ratée laisse un objet derrière elle.
			glDeleteShader(vs);
			return false;
		}

		const GLuint prog = glCreateProgram();
		glAttachShader(prog, vs);
		glAttachShader(prog, fs);
		glLinkProgram(prog);

		// L'édition de liens a son propre journal, distinct de celui de la
		// compilation. Deux nuanceurs qui compilent chacun très bien peuvent
		// refuser de se lier : c'est le cas quand une sortie du premier ne
		// correspond pas à l'entrée du second, en nom ou en type.
		GLint ok = GL_FALSE;
		glGetProgramiv(prog, GL_LINK_STATUS, &ok);
		if (ok == GL_FALSE) {
			GLint taille = 0;
			glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &taille);
			nkentseu::NkVector<char> journal((size_t)(taille > 1 ? taille : 1), '\0');
			glGetProgramInfoLog(prog, taille, nullptr, journal.Data());
			mErreur = nkentseu::NkString("edition de liens :\n") + journal.Data();

			glDeleteProgram(prog);
			glDeleteShader(vs);
			glDeleteShader(fs);
			return false;
		}

		// Une fois liés, les deux nuanceurs ne servent plus : le programme en
		// détient une copie. Les détacher puis les supprimer libère leur code
		// source côté pilote. Beaucoup de programmes ne le font jamais et
		// gardent des mégaoctets de GLSL en mémoire graphique.
		glDetachShader(prog, vs);
		glDetachShader(prog, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);

		mProgramme = prog;
		return true;
	}

	void Nuanceur::Detruire() {
		if (mProgramme != 0) {
			glDeleteProgram(mProgramme);
			mProgramme = 0;
		}
	}

	void Nuanceur::Activer() const {
		glUseProgram(mProgramme);
	}

	int Nuanceur::Emplacement(const char *nom) const {
		const int *trouve = mEmplacements.Find(nkentseu::NkString(nom));
		if (trouve != nullptr) {
			++mCache;
			return *trouve;
		}

		++mRecherches;
		const int e = (int)glGetUniformLocation(mProgramme, nom);
		mEmplacements.Insert(nkentseu::NkString(nom), e);
		return e;
	}

	void Nuanceur::PoserVec2(const char *nom, float x, float y) const {
		const int e = Emplacement(nom);
		if (e >= 0)
			glUniform2f(e, x, y);
	}

	void Nuanceur::PoserVec3(const char *nom, float x, float y, float z) const {
		const int e = Emplacement(nom);
		if (e >= 0)
			glUniform3f(e, x, y, z);
	}

} // namespace monrhi