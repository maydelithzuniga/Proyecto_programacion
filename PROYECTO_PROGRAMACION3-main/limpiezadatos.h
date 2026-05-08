//
// Created by maydelithzuniga on 07/05/2026.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

class limpiezadatos {
private:
    string archivo_entrada="wiki_movie_plots_deduped.csv";
    string archivo_final="wiki_movie_plots_deduped_final.csv";
public:
    limpiezadatos()=default;
    ~limpiezadatos()=default;
    // función trim
    string trim(const string& s) {
        size_t inicio = s.find_first_not_of(" \t\r\n");
        if (inicio == string::npos) return ""; // solo espacios → vacío
        size_t fin = s.find_last_not_of(" \t\r\n");
        return s.substr(inicio, fin - inicio + 1);
    }
    string limpiar_espacio(const string& s) {
        string resultado = s;
        size_t pos = 0;
        while ((pos = resultado.find("\r\n", pos)) != string::npos) {
            resultado.replace(pos, 2, " ");
        }
        pos = 0;
        while ((pos = resultado.find("\r", pos)) != string::npos) {
            resultado.replace(pos, 1, " ");
        }
        return resultado;
    }

    void limpiardatoscsv() {
        ifstream entrada(archivo_entrada);
        ofstream salida(archivo_final);
        if (!entrada.is_open() || !salida.is_open()) {
            cout << "Error al abrir archivos" << endl;
            return;
        }

        string linea;

        while (getline(entrada, linea)) {
            vector<string> fila_procesada;
            string celda;
            bool en_comillas = false;
            for (char c : linea) {
                if (c == '"') {
                    en_comillas = !en_comillas;
                    celda += c;
                }
                else if (c == ',' && !en_comillas) {
                    celda= trim(celda);
                    if (celda == "" || celda == "Unknown" || celda == "\"\"" || celda == "\"Unknown\"") {
                        celda = "unknown";
                    }
                    celda=limpiar_espacio(celda);
                    fila_procesada.push_back(celda);
                    celda.clear();
                }
                else {
                    celda += c;
                }
            }
            celda=trim(celda);
            if (celda == "" || celda == "Unknown" || celda == "\"\"" || celda == "\"Unknown\"") {
                celda = "unknown";
            }
            celda=limpiar_espacio(celda);
            fila_procesada.push_back(celda);


            for (size_t i = 0; i < fila_procesada.size(); i++) {
                salida << fila_procesada[i];
                if (i != fila_procesada.size() - 1) {
                    salida << ",";
                }
            }
            salida << "\n";
        }

        entrada.close();
        salida.close();

        cout << "Archivo _copy generado correctamente." << endl;

    }
};