// Maillage : un tampon de sommets, avec ou sans tampon d'index.
#include "MonRhi/Maillage.h"
#include "MonRhi/Gl.h"

#include <cstddef>

namespace monrhi {

	namespace {
		// La recette de lecture : position en 0, couleur en 1.
		// Le tableau (VAO) et le tampon de sommets doivent deja etre lies.
		void PoserRecette() {
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(Sommet),
								  (const void *)offsetof(Sommet, x));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(Sommet),
								  (const void *)offsetof(Sommet, r));
			glEnableVertexAttribArray(1);
		}
	} // namespace

	Maillage::~Maillage() {
		Detruire();
	}

	bool Maillage::Creer(const Sommet *sommets, int nombre) {
		Detruire();
		if (sommets == nullptr || nombre <= 0)
			return false;

		glGenVertexArrays(1, &mVao);
		if (mVao == 0)
			return false;
		glBindVertexArray(mVao);

		glGenBuffers(1, &mVbo);
		glBindBuffer(GL_ARRAY_BUFFER, mVbo);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(Sommet) * (size_t)nombre), sommets,
					 GL_STATIC_DRAW);

		PoserRecette();

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		mNombre = nombre;
		return true;
	}

	bool Maillage::CreerIndexe(const Sommet *sommets, int nombreSommets,
							   const unsigned short *index, int nombreIndex) {
		Detruire();
		if (sommets == nullptr || nombreSommets <= 0 || index == nullptr || nombreIndex <= 0)
			return false;

		// Comme Creer, jusqu'aux attributs : le tableau est lie en premier.
		glGenVertexArrays(1, &mVao);
		if (mVao == 0)
			return false;
		glBindVertexArray(mVao);

		glGenBuffers(1, &mVbo);
		glBindBuffer(GL_ARRAY_BUFFER, mVbo);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(Sommet) * (size_t)nombreSommets), sommets,
					 GL_STATIC_DRAW);

		PoserRecette();

		// Le tampon d'index, tableau encore lie : c'est le tableau qui le retient.
		glGenBuffers(1, &mEbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					 (GLsizeiptr)(sizeof(unsigned short) * (size_t)nombreIndex),
					 index, GL_STATIC_DRAW);

		// On delie le TABLEAU d'abord. Si on deliait GL_ELEMENT_ARRAY_BUFFER
		// avant, on effacerait ce qu'on vient d'enregistrer. Et on ne delie
		// jamais le tampon d'index : le tableau le perdrait, sans message.
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		mNombre = nombreIndex;
		return true;
	}

	void Maillage::Detruire() {
		if (mEbo != 0) {
			glDeleteBuffers(1, &mEbo);
			mEbo = 0;
		}
		if (mVbo != 0) {
			glDeleteBuffers(1, &mVbo);
			mVbo = 0;
		}
		if (mVao != 0) {
			glDeleteVertexArrays(1, &mVao);
			mVao = 0;
		}
		mNombre = 0;
	}

	void Maillage::Dessiner() const {
		if (mVao == 0)
			return;
		glBindVertexArray(mVao);
		if (mEbo != 0) {
			glDrawElements(GL_TRIANGLES, (GLsizei)mNombre, GL_UNSIGNED_SHORT, (const void *)0);
		} else {
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)mNombre);
		}
		glBindVertexArray(0);
	}

} // namespace monrhi