#include "Vistas.h"
#include "MotorBusqueda.h"
#include "limpiezadatos.h"
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

int main() {
    // ── 1. Limpieza y generación del CSV final ────────────────────────────────


    // ── 2. Carga de la base de datos ──────────────────────────────────────────
    EstadoPantalla estado = MENU_PRINCIPAL;
    std::unordered_map<int, Pelicula> db = cargarCSV();
    if (db.empty()) {
        std::cout << "No se pudo cargar la base de datos.\n";
        std::cout << "Verifica que el archivo CSV este en la carpeta del proyecto.\n";
        return 1;
    }

    // ── 3. Estado de la aplicación ────────────────────────────────────────────
    std::vector<Pelicula> resultados;
    std::vector<int> likes;
    std::vector<int> verMasTarde;
    std::string ultimaBusqueda;
    bool ultimaBusquedaPorTag = false;

    // ── 4. Bucle principal ────────────────────────────────────────────────────
    while (estado != SALIR) {
        switch (estado) {
            case MENU_PRINCIPAL:
                estado = vistaMenuPrincipal();
                break;

            case BUSCAR: {
                std::string consulta;
                bool esBusquedaPorTag = false;
                estado = vistaBuscar(consulta, esBusquedaPorTag);
                if (!consulta.empty()) {
                    if (esBusquedaPorTag) {
                        resultados = buscarPorTag(db, consulta);
                    } else {
                        resultados = buscarPorPalabra(db, consulta);
                    }
                    resultados = top5(resultados, consulta);
                    ultimaBusqueda = consulta;
                }
                break;
            }

            case RESULTADOS:
                estado = vistaResultados(resultados, likes, verMasTarde);
                break;

            case VER_MAS_TARDE:
                estado = vistaVerMasTarde(verMasTarde, db, likes);
                break;

            case PELICULAS_LIKE:
                estado = vistaPeliculasLike(likes, db);
                break;

            case PELICULAS_RECOMENDADA:
                estado = vistaPeliculasRecomendada(db, likes);
                break;

            default:
                estado = SALIR;
                break;
        }
    }

    limpiarPantalla();
    std::cout << "Hasta luego!\n";
    return 0;
}