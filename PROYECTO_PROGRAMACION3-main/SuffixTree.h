#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

// ── Nodo del Árbol de Sufijos Comprimido ──────────────────────────────────────
struct NodoSufijo {
    std::unordered_map<char, std::shared_ptr<NodoSufijo>> hijos;
    std::string etiqueta;          // etiqueta de la arista hacia este nodo
    std::vector<int> movie_ids;    // IDs de películas que pasan por este nodo

    NodoSufijo() = default;
    explicit NodoSufijo(const std::string& lbl) : etiqueta(lbl) {}
};

// ── Árbol de Sufijos Comprimido (Patricia Trie) ───────────────────────────────
class SuffixTree {
public:
    SuffixTree() : raiz(std::make_shared<NodoSufijo>()) {}

    // Inserta todos los sufijos de 'token' asociados a 'movie_id'
    inline void insertar(const std::string& token, int movie_id) {
        for (size_t i = 0; i < token.size(); ++i)
            _insertar(raiz, token.substr(i), movie_id);
    }

    // Devuelve IDs únicos de películas cuya clave contiene 'patron' como prefijo
    inline std::vector<int> buscar(const std::string& patron) const {
        std::vector<int> resultado;
        std::shared_ptr<NodoSufijo> nodo = _navegar(raiz, patron);
        if (nodo) _recolectar(nodo, resultado);
        _deduplicar(resultado);
        return resultado;
    }

private:
    std::shared_ptr<NodoSufijo> raiz;

    // ── Inserción recursiva con compresión de caminos ─────────────────────────
    inline void _insertar(std::shared_ptr<NodoSufijo> nodo,
                           const std::string& sufijo,
                           int movie_id) {
        if (sufijo.empty()) return;

        char primera = sufijo[0];

        // No existe hijo con ese primer caracter: crear nodo hoja directo
        if (nodo->hijos.find(primera) == nodo->hijos.end()) {
            auto hoja = std::make_shared<NodoSufijo>(sufijo);
            hoja->movie_ids.push_back(movie_id);
            nodo->hijos[primera] = hoja;
            return;
        }

        auto hijo = nodo->hijos[primera];
        size_t coincide = _prefijo_comun(hijo->etiqueta, sufijo);

        // El sufijo coincide exactamente con la etiqueta: anotar ID y terminar
        if (coincide == sufijo.size() && coincide == hijo->etiqueta.size()) {
            _anotar(hijo, movie_id);
            return;
        }

        // El sufijo está contenido en la etiqueta: dividir (split)
        if (coincide == sufijo.size()) {
            auto nuevo = std::make_shared<NodoSufijo>(sufijo);
            nuevo->movie_ids.push_back(movie_id);
            hijo->etiqueta = hijo->etiqueta.substr(coincide);
            nuevo->hijos[hijo->etiqueta[0]] = hijo;
            nodo->hijos[primera] = nuevo;
            return;
        }

        // La etiqueta está contenida en el sufijo: bajar por el hijo
        if (coincide == hijo->etiqueta.size()) {
            _anotar(hijo, movie_id);
            _insertar(hijo, sufijo.substr(coincide), movie_id);
            return;
        }

        // Coincidencia parcial: split + dos nuevos hijos
        auto intermedio = std::make_shared<NodoSufijo>(sufijo.substr(0, coincide));
        _anotar(intermedio, movie_id);

        hijo->etiqueta = hijo->etiqueta.substr(coincide);
        intermedio->hijos[hijo->etiqueta[0]] = hijo;

        auto nueva_hoja = std::make_shared<NodoSufijo>(sufijo.substr(coincide));
        nueva_hoja->movie_ids.push_back(movie_id);
        intermedio->hijos[nueva_hoja->etiqueta[0]] = nueva_hoja;

        nodo->hijos[primera] = intermedio;
    }

    // ── Navega el árbol consumiendo 'patron'; devuelve nodo de llegada ─────────
    inline std::shared_ptr<NodoSufijo> _navegar(
            std::shared_ptr<NodoSufijo> nodo,
            const std::string& patron) const {
        if (patron.empty()) return nodo;

        char primera = patron[0];
        auto it = nodo->hijos.find(primera);
        if (it == nodo->hijos.end()) return nullptr;

        auto hijo = it->second;
        size_t coincide = _prefijo_comun(hijo->etiqueta, patron);

        if (coincide < patron.size() && coincide == hijo->etiqueta.size())
            return _navegar(hijo, patron.substr(coincide));

        if (coincide >= patron.size())
            return hijo;           // patrón agotado dentro de una etiqueta

        return nullptr;            // divergencia: no existe
    }

    // ── Recolecta todos los movie_ids del subárbol rooted en 'nodo' ───────────
    inline void _recolectar(std::shared_ptr<NodoSufijo> nodo,
                             std::vector<int>& out) const {
        for (int id : nodo->movie_ids) out.push_back(id);
        for (auto& [_, hijo] : nodo->hijos) _recolectar(hijo, out);
    }

    // ── Helpers ───────────────────────────────────────────────────────────────
    inline static size_t _prefijo_comun(const std::string& a, const std::string& b) {
        size_t i = 0;
        while (i < a.size() && i < b.size() && a[i] == b[i]) ++i;
        return i;
    }

    inline static void _anotar(std::shared_ptr<NodoSufijo> nodo, int movie_id) {
        if (std::find(nodo->movie_ids.begin(), nodo->movie_ids.end(), movie_id)
            == nodo->movie_ids.end())
            nodo->movie_ids.push_back(movie_id);
    }

    inline static void _deduplicar(std::vector<int>& v) {
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
    }
};