#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS 
#include "structures_solutionnaire_td3.hpp"    
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <sstream>
#include <forward_list>
#include "cppitertools/range.hpp"
#include "cppitertools/enumerate.hpp"
#include "gsl/span"
#include <set>
#include <algorithm>
#include <numeric>


#if __has_include("gtest/gtest.h")
#include "gtest/gtest.h"
#endif

#if __has_include("bibliotheque_cours.hpp")
#include "bibliotheque_cours.hpp"
#define BIBLIOTHEQUE_COURS_INCLUS
using bibliotheque_cours::cdbg;
#else
auto& cdbg = clog;
#endif

#if __has_include("verification_allocation.hpp")
#include "verification_allocation.hpp"
#include "debogage_memoire.hpp"  
#endif

void initialiserBibliothequeCours([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
#ifdef BIBLIOTHEQUE_COURS_INCLUS
	bibliotheque_cours::activerCouleursAnsi();
	bibliotheque_cours::executerGoogleTest(argc, argv);
#endif
}

using namespace std;
using namespace iter;
using namespace gsl;

#pragma endregion

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{
template <typename T>
T lireType(istream& fichier)
{
	T valeur{};
	fichier.read(reinterpret_cast<char*>(&valeur), sizeof(valeur));
	return valeur;
}
#define erreurFataleAssert(message) assert(false&&(message)),terminate()
static const uint8_t enteteTailleVariableDeBase = 0xA0;
size_t lireUintTailleVariable(istream& fichier)
{
	uint8_t entete = lireType<uint8_t>(fichier);
	switch (entete) {
	case enteteTailleVariableDeBase + 0: return lireType<uint8_t>(fichier);
	case enteteTailleVariableDeBase + 1: return lireType<uint16_t>(fichier);
	case enteteTailleVariableDeBase + 2: return lireType<uint32_t>(fichier);
	default:
		erreurFataleAssert("Tentative de lire un entier de taille variable alors que le fichier contient autre chose à cet emplacement.");
	}
}

string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUintTailleVariable(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion/
void ListeFilms::changeDimension(int nouvelleCapacite)
{
	Film** nouvelleListe = new Film * [nouvelleCapacite];

	if (elements != nullptr) {
		nElements = min(nouvelleCapacite, nElements);
		for (int i : range(nElements))
			nouvelleListe[i] = elements[i];
		delete[] elements;
	}

	elements = nouvelleListe;
	capacite = nouvelleCapacite;
}

void ListeFilms::ajouterFilm(Film* film)
{
	if (nElements == capacite)
		changeDimension(max(1, capacite * 2));
	elements[nElements++] = film;
}

span<Film*> ListeFilms::enSpan() const { return span(elements, nElements); }

shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const
{
	for (const Film* film : enSpan()) {
		for (const shared_ptr<Acteur>& acteur : film->acteurs.enSpan()) {
			if (acteur->nom == nomActeur)
				return acteur;
		}
	}
	return nullptr;
}


shared_ptr<Acteur> lireActeur(istream& fichier, const ListeFilms& listeFilms)
{
	Acteur acteur = {};
	acteur.nom = lireString(fichier);
	acteur.anneeNaissance = int(lireUintTailleVariable(fichier));
	acteur.sexe = char(lireUintTailleVariable(fichier));

	shared_ptr<Acteur> acteurExistant = listeFilms.trouverActeur(acteur.nom);
	if (acteurExistant != nullptr)
		return acteurExistant;
	else {
		cout << "Création Acteur " << acteur.nom << endl;
		return make_shared<Acteur>(move(acteur));  // Le move n'est pas nécessaire mais permet de transférer le texte du nom sans le copier.
	}
}

Film* lireFilm(istream& fichier, ListeFilms& listeFilms)
{
	Film* film = new Film;
	film->titre = lireString(fichier);
	film->realisateur = lireString(fichier);
	film->anneeSortie = int(lireUintTailleVariable(fichier));
	film->recette = int(lireUintTailleVariable(fichier));
	auto nActeurs = int(lireUintTailleVariable(fichier));
	film->acteurs = ListeActeurs(nActeurs);
	cout << "Création Film " << film->titre << endl;

	for ([[maybe_unused]] auto i : range(nActeurs)) {
		film->acteurs.ajouter(lireActeur(fichier, listeFilms));
	}

	return film;
}

ListeFilms creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nElements = int(lireUintTailleVariable(fichier));

	ListeFilms listeFilms;
	for ([[maybe_unused]] int i : range(nElements)) {
		listeFilms.ajouterFilm(lireFilm(fichier, listeFilms));
	}

	return listeFilms;
}

void ListeFilms::detruire()
{

	delete[] elements;
}

ostream& operator<< (ostream& os, const Acteur& acteur)
{
	return os << "  " << acteur.nom << ", " << acteur.anneeNaissance << " " << acteur.sexe << endl;
}

ostream& operator<< (ostream& os, const Affichable& affichable)
{
	affichable.afficherSur(os);
	return os;
}

void Item::afficherSur(ostream& os) const
{
	os << "Titre: " << titre << "  Année:" << anneeSortie << endl;
}

void Film::afficherSpecifiqueSur(ostream& os) const
{
	os << "  Réalisateur: " << realisateur << endl;
	os << "  Recette: " << recette << "M$" << endl;
	os << "Acteurs:" << endl;
	for (auto&& acteur : acteurs.enSpan())
		os << *acteur;
}

void Film::afficherSur(ostream& os) const
{
	Item::afficherSur(os);
	Film::afficherSpecifiqueSur(os);
}

void Livre::afficherSpecifiqueSur(ostream& os) const
{
	os << "  Auteur: " << auteur << endl;
	os << "  Vendus: " << copiesVendues << "M  Pages: " << nPages << endl;
}

void Livre::afficherSur(ostream& os) const
{
	Item::afficherSur(os);
	Livre::afficherSpecifiqueSur(os);
}


void FilmLivre::afficherCourt(ostream& os) const {
	Film::afficherCourt(os);
	os << " / ";
	Livre::afficherCourt(os);
}

void Item::lireDe(istream& is)
{
	is >> quoted(titre) >> anneeSortie;
}
void Livre::lireDe(istream& is)
{
	Item::lireDe(is);
	is >> quoted(auteur) >> copiesVendues >> nPages;
}
Livre::Livre(istream& is) {
	lireDe(is);
}

template <class T>
void afficherListeItems(span<unique_ptr<T>> listeItems)
{
	static const string ligneDeSeparation = "\033[32m────────────────────────────────────────\033[0m\n";
	cout << ligneDeSeparation;
	for (auto&& item : listeItems) {
		item->afficherCourt(cout);
		cout << ligneDeSeparation;
	}
}

///////////////////// 3.1 ///////////////////////

template<typename InputIt, typename OutputIt, typename UnaryPredicate>
OutputIt copy_range(InputIt first, InputIt last, OutputIt d_first, UnaryPredicate pred) {
	return std::copy_if(first, last, d_first, pred);
}


int main(int argc, char* argv[])
{
	initialiserBibliothequeCours(argc, argv);

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	vector<unique_ptr<Item>> items;
	{
		{
			///////////////////// 1.5 ///////////////////////
			ListeFilms listeFilms = creerListe("films.bin");
			cout << "les acteurs de Aliens sont: " << endl;
			cout << ligneDeSeparation << endl;
			
			
			Film* ptrAlien = listeFilms[0];
			for (auto&& acteur : ptrAlien->getActeurs()) {
				cout << acteur->nom << endl;
			};
			cout << ligneDeSeparation << endl;


			for (auto&& film : listeFilms.enSpan())
				items.push_back(unique_ptr<Item>(film));
				
		}
	
	
			

		{
			ifstream fichier("livres.txt");
			fichier.exceptions(ios::failbit);
			while (!ws(fichier).eof())
				items.push_back(make_unique<Livre>(fichier));
		}
	}

	items.push_back(make_unique<FilmLivre>(dynamic_cast<Film&>(*items[4]), dynamic_cast<Livre&>(*items[9])));  // On ne demandait pas de faire une recherche; serait direct avec la matière du TD5.

	afficherListeItems(span(items));


	////////////////////	1.1	//////////////////////////////

	forward_list<unique_ptr<Item>> forwardItemList;

	for (auto i = items.rbegin(); i != items.rend(); ++i) {		//rend: pointe vers le debut du conteneur -> ie: range de la fin au debut
		if (*i) {
		forwardItemList.emplace_front((*i).get());				// emplace front: met les elements au debut un a la suite des autres, en commencant par les elements de la fi 
		}
	}
	

	//// /////////////////// 1.2 /////////////////////////////

	forward_list<unique_ptr<Item>> reversedList;

	for (auto i = items.begin(); i != items.end(); ++i) {
		reversedList.emplace_front((*i).get());
	}

	///////////////////// 1.3 ///////////////////////

	forward_list<unique_ptr<Item>> copieFowardList;

	for (auto i = forwardItemList.begin(); i != forwardItemList.end(); ++i) {
		copieFowardList.emplace_front((*i).get());
	}


	///////////////////// 1.4: ordre = O(n) ///////////////////////

	vector<unique_ptr<Item>> reversedVector;
	reversedVector.reserve(items.size());

	for (auto it = items.begin(); it != items.end(); ++it) {
		reversedVector.emplace(reversedVector.begin(), (*it).get());
	};

	/////////////////////// 2.1 ///////////////////////


// 	struct ItemPtrComparer {
// 		bool operator()(const Item* a, const Item* b) const {
// 			return a->getTitre() < b->getTitre();
// 		}
// 	};


// 	// Déclaration du conteneur set avec le comparateur personnalisé
// 	set<Item*, ItemPtrComparer> itemsSet;
// 	cout << "1" << endl;

// 	// Ajout des pointeurs d'Item dans le set
// 	for (const auto& itemPtr : items) {
// 		cout << "2" << endl;
// 		itemsSet.insert(itemPtr.get());
// 		cout << "2" << endl;

// 	}


// 	// Affichage des éléments triés par titre
// 	std::cout << "Items triés par titre :" << std::endl;
// 	for (const auto& itemPtr : itemsSet) {
// 		itemPtr->afficherCourt(std::cout);
// 		std::cout << std::endl;
// 	}

// 		cout << "2.1 bon" << endl;

// 	// ///////////////////// 2.2 ///////////////////////

// 	//  // Déclaration et initialisation du conteneur de hachage
//     // std::unordered_map<std::string, const Item*> itemsMap;
//     // for (const auto& item : items) {
//     //     itemsMap[item->getTitre()] = item.get();
//     // }

//     // // Recherche de l'item "The Hobbit" dans le conteneur de hachage
//     // std::string titreCherche = "The Hobbit";
//     // auto it = itemsMap.find(titreCherche);

//     // // Vérification si l'item a été trouvé
//     // if (it != itemsMap.end()) {
//     //     std::cout << "Item trouvé : " << it->second->getTitre() << std::endl;
//     // } else {
//     //     std::cout << "Item non trouvé." << std::endl;
//     // }

// 	// ///////////////////// 3.1 ///////////////////////


// 	// //Vecteur pour stocker les films
// 	// std::vector<unique_ptr<Film>> films;

// 	// [](const unique_ptr<Item>& item) {
// 	// 	return unique_ptr<Film>(dynamic_cast<Film*>(item.get()));
// 	// 	};

// 	// /////////////////// 3.2 ///////////////////////

// 	// int sommeRecettes = accumulate(films.begin(), films.end(), 0,
// 	// 	[](int acc, const unique_ptr<Film>& film) {
// 	// 		return acc + film->getRecette(); 
// 	// 	});

// 	// std::cout << "La somme des recettes des films est : " << sommeRecettes << std::endl;
// }

}