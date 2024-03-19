// Solutionnaire du TD4 INF1015 hiver 2024
// Par Francois-R.Boyer@PolyMtl.ca

#pragma once
// Structures mémoires pour une collection de films.
#include <forward_list>
#include <string>
#include <iostream>
#include <memory>
#include <functional>
#include <cassert>
#include "gsl/span"
using gsl::span;
using namespace std;

class Film; struct Acteur; 

class ListeFilms {
public:
	ListeFilms() = default;
	void ajouterFilm(Film* film);
	shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	int capacity() const { return capacite; } 
	void detruire();
	Film*& operator[] (int index) { assert(0 <= index && index < nElements);  return elements[index]; }
	Film* trouver(const function<bool(const Film&)>& critere) {
		for (auto& film : enSpan())
			if (critere(*film))
				return film;
		return nullptr;
	}

private:
	void changeDimension(int nouvelleCapacite);

	int capacite = 0, nElements = 0;
	Film** elements = nullptr; 
};

template <typename T>
class Liste {
public:
	Liste() = default;
	explicit Liste(int capaciteInitiale) :  // explicit n'est pas matière à ce TD, mais c'est un cas où c'est bon de l'utiliser, pour ne pas qu'il construise implicitement une Liste à partir d'un entier, par exemple "maListe = 4;".
		capacite_(capaciteInitiale),
		elements_(make_unique<shared_ptr<T>[]>(capacite_))
	{
	}
	Liste(const Liste<T>& autre) :
		capacite_(autre.nElements_),
		nElements_(autre.nElements_),
		elements_(make_unique<shared_ptr<T>[]>(nElements_))
	{
		for (int i = 0; i < nElements_; ++i)
			elements_[i] = autre.elements_[i];
	}
	//NOTE: On n'a pas d'operator= de copie, ce n'était pas nécessaire pour répondre à l'énoncé. On aurait facilement pu faire comme dans les notes de cours et définir l'operator= puis l'utiliser dans le constructeur de copie.
	//NOTE: Nos constructeur/operator= de move laissent la liste autre dans un état pas parfaitement valide; il est assez valide pour que la destruction et operator= de move fonctionnent, mais il ne faut pas tenter d'ajouter, de copier ou d'accéder aux éléments de cette liste qui "n'existe plus". Normalement le move sur les classes de la bibliothèque standard C++ laissent les objets dans un "valid but unspecified state" (https://en.cppreference.com/w/cpp/utility/move). Pour que l'état soit vraiment valide, on devrait remettre à zéro la capacité et nombre d'éléments de l'autre liste.
	Liste(Liste<T>&&) = default;  // Pas nécessaire, mais puisque c'est si simple avec unique_ptr...
	Liste<T>& operator= (Liste<T>&&) noexcept = default;  // Utilisé pour l'initialisation dans lireFilm.

	void ajouter(shared_ptr<T> element)
	{
		assert(nElements_ < capacite_);  // Comme dans le TD1, on ne demande pas la réallocation pour ListeActeurs...
		elements_[nElements_++] = move(element);
	}

	// Noter que ces accesseurs const permettent de modifier les éléments; on pourrait vouloir des versions const qui retournent des const shared_ptr, et des versions non const qui retournent des shared_ptr.  En C++23 on pourrait utiliser "deducing this".
	shared_ptr<T>& operator[] (int index) const { assert(0 <= index && index < nElements_); return elements_[index]; }
	span<shared_ptr<T>> enSpan() const { return span(elements_.get(), nElements_); }

private:
	int capacite_ = 0, nElements_ = 0;
	unique_ptr<shared_ptr<T>[]> elements_;
};

using ListeActeurs = Liste<Acteur>;

class Affichable {
public:
	virtual void afficherCourt(ostream& os) const = 0;
	virtual void afficherSur(ostream& os) const = 0;
	virtual ~Affichable() = default;

};

class Item : public Affichable {
public:
	void afficherSur(ostream& os) const override;
	void lireDe(istream& is);

	friend Film* lireFilm(istream& fichier, ListeFilms& listeFilms);

	virtual void afficherCourt(ostream& os) const override {
        os << titre;
    }

protected:
    string getTitre() const { return titre; }


private:
	string titre;
	int anneeSortie = 0;
};




class Film : virtual public Item
{
public:
	void afficherCourt(ostream& os) const override {
			os << getTitre() << ", par " << realisateur;
		}
	void afficherSur(ostream& os) const override;
	void afficherSpecifiqueSur(ostream& os) const;  

	friend Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
	friend shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const;
	template <typename T> struct accessible_pour_tests_par;  

	span<shared_ptr<Acteur>> getActeurs() const {
        return acteurs.enSpan();
    }


protected:
	string realisateur; 

private:
	int recette = 0;
	ListeActeurs acteurs;
};




class Livre : virtual public Item
{
public:
	void afficherCourt(ostream& os) const override {
			os << getTitre() << ", par " << auteur;
		}

	Livre() = default;
	explicit Livre(istream& is);
	void afficherSur(ostream& os) const override;
	void afficherSpecifiqueSur(ostream& os) const;
	void lireDe(istream& is);

protected:
	string auteur;

private:
	int copiesVendues=0, nPages=0;
};




class FilmLivre : public Film, public Livre {
public:
	FilmLivre(const Film& film, const Livre& livre) : Item(film), Film(film), Livre(livre) { }

	void afficherSur(ostream& os) const override {
        Film::afficherCourt(os);
        os << " / ";
        Livre::afficherCourt(os);
    }
	void afficherCourt(ostream& os) const override;
};



struct Acteur
{
	string nom; int anneeNaissance=0; char sexe='\0';
};
