# Programación III: Proyecto Final

# Netflix: Buscador inteligente

# Integrantes:
* Maydelith Zuñiga
* Joaquin Llallire
* 
* sdkaop

## Preprocesamiento de Datos

El dataset utilizado es `wiki_movie_plots_deduped.csv`, un archivo CSV con información de películas extraída de Wikipedia (34,886 filas). Antes de ser usado por el sistema, pasa por una etapa de limpieza implementada en la clase `limpiezadatos`.

### ¿Qué limpia?

El procesamiento recorre el archivo **línea por línea** sin cargarlo en memoria, parseando manualmente cada celda respetando campos entre comillas que pueden contener comas internas (como ocurre en los plots).

Por cada celda aplica tres limpiezas:

- **Espacios y tabs en los extremos** — elimina espacios, tabs y saltos de línea al inicio y final de cada celda
- **Valores desconocidos** — normaliza celdas vacías o con valor `Unknown` en cualquier variante a `unknown` en minúsculas
- **Saltos de línea de Windows** — elimina caracteres `\r\n` y `\r` que aparecen dentro del texto de los plots

### Resumen de transformaciones

| Valor original | Resultado |
|---|---|
| *(vacío)* o `"   "` | `unknown` |
| `Unknown` / `"Unknown"` | `unknown` |
| `texto\r\ntexto` | `texto texto` |
| Cualquier otro valor | Sin cambios |

## Preprocesamiento de datos — De CSV a tokens

El preprocesamiento transforma el archivo CSV crudo en palabras listas para ser insertadas en el árbol de sufijos. Se realiza en tres etapas.

---

### Etapa 1 — Carga y estructuración

`cargarCSV()` lee el archivo `wiki_movie_plots_deduped_final.csv` línea por línea y construye un vector de objetos `Pelicula`. Cada objeto almacena los campos de la película ya separados:

```
Release Year,Title,Director,Cast,Genre,Plot
      ↓
Pelicula {
    id, anio, titulo, sinopsis,
    directores → ["Steven Spielberg"]
    actores    → ["Tom Hanks", "Robin Wright"]
    generos    → ["drama", "romance"]
}
```

Como la sinopsis puede ocupar varias líneas en el CSV, el cargador detecta el inicio de cada película por el patrón de año al inicio de línea (`1995,...`) y acumula todas las líneas que le pertenecen antes de parsearla.

---

### Etapa 2 — Limpieza y tokenización

Una vez construido el objeto `Pelicula`, cada campo de texto pasa por dos funciones antes de llegar al árbol:

**`limpiar(texto)`** — normaliza el texto para que las comparaciones sean consistentes. Convierte todo a minúsculas y elimina puntuación como `.`, `,`, `(`, `)`, `"`, `'`, `-`, entre otros.

```
"Fight! He runs." → "fight he runs"
```

**`tokenizar(texto)`** — divide el texto limpio en palabras individuales usando espacios como separador.

```
"fight he runs" → ["fight", "he", "runs"]
```

---

### Etapa 3 — Inserción al árbol

Con los tokens listos, `construirArbol()` los inserta en el `SuffixTree` asociando cada token al `id` de su película. Esto se hace para todos los campos buscables:

```
pelicula.id = 42
tokenizar(titulo)   → ["kansas", "saloon"]      → arbol.insertar("kansas", 42)
tokenizar(sinopsis) → ["bartender", "beer", ...] → arbol.insertar("bartender", 42)
tokenizar(director) → ["spielberg"]              → arbol.insertar("spielberg", 42)
tokenizar(actores)  → ["tom", "hanks"]           → arbol.insertar("tom", 42)
tokenizar(generos)  → ["drama"]                  → arbol.insertar("drama", 42)
```

> El árbol nunca almacena objetos `Pelicula`, solo enteros. Cuando el usuario busca, el árbol devuelve IDs y con esos IDs se recuperan las películas del vector original.

---

### Flujo completo

```
CSV → cargarCSV() → vector<Pelicula>
                          ↓
                   construirArbol()
                          ↓
                   limpiar() + tokenizar()
                          ↓
                   arbol.insertar(token, id)
                          ↓
                   árbol listo para búsqueda
```

## Procesamiento de la consulta del usuario

Antes de realizar la búsqueda en el árbol, la consulta ingresada por el usuario pasa por un proceso de normalización para garantizar que las palabras sean comparables con los tokens almacenados en la estructura. El pseudocódigo de este proceso es el siguiente:

```
INICIO

    // 1. usuario escribe en consola
    consultaUsuario ← leerTexto()
    // ejemplo: "Steven Spielberg"

    // 2. convertir a minúsculas y eliminar puntuación
    textoLimpio ← limpiar(consultaUsuario)
    // "Steven Spielberg" → "steven spielberg"

    // 3. dividir en palabras individuales
    tokens ← tokenizar(textoLimpio)
    // "steven spielberg" → ["steven", "spielberg"]

    // 4. cada token entra al árbol
    PARA CADA token EN tokens:
        arbol.buscar(token)

FIN
```

##  Estructura de Datos — Árbol de Sufijos Comprimido (Patricia Trie)

La estructura de datos escogida es un **Árbol de Sufijos Comprimido**, también conocido como variante de **Patricia Trie** aplicada a sufijos. Fue elegida porque el proyecto requiere buscar películas no solo por palabras completas, sino también por sub-palabras. Por ejemplo, si el usuario busca `bar`, el programa encuentra películas que contengan palabras como `barco`, `barca` o cualquier palabra que contenga ese patrón.

### ¿Cómo se insertan las palabras?

Para lograrlo, el árbol no inserta únicamente la palabra completa sino **todos sus sufijos**. Por ejemplo, para la palabra `barco` se insertan:

```txt
barco
arco
rco
co
o
```

Esto garantiza que cualquier búsqueda por subcadena encuentre un match, porque esa subcadena aparecerá como prefijo de alguno de los sufijos insertados.

### Compresión de caminos

A diferencia de un Trie clásico que crea un nodo por letra, el Patricia Trie **comprime secuencias de nodos en una sola arista con etiqueta**. Esto reduce drásticamente el uso de memoria:
Trie clásico:        Patricia Trie:
b                    "barco" → movie_id: 42
└─ a
└─ r
└─ c
└─ o  → movie_id: 42

Cuando dos palabras comparten un prefijo, el nodo se divide exactamente en el punto de divergencia:
Insertar "barco" y "barca":
"bar"
├─ "co" → movie_id: 42
└─ "ca" → movie_id: 17

### Cada nodo guarda IDs, no objetos

Los nodos almacenan únicamente **enteros** que representan el `id` de cada película. Al recuperar resultados, esos IDs se usan para acceder directamente al catálogo `unordered_map<int, Pelicula>`, donde cada película existe una sola vez sin importar cuántos nodos del árbol la referencien.
nodo "barco" → [42, 103, 987]
↓
catalogo[42]  → Pelicula { titulo, sinopsis, ... }
catalogo[103] → Pelicula { titulo, sinopsis, ... }
catalogo[987] → Pelicula { titulo, sinopsis, ... }

### Rendimiento de búsqueda

| | Búsqueda lineal | Árbol de Sufijos |
|---|---|---|
| Recorre todo el dataset | ✅ siempre | ❌ nunca |
| Tiempo de búsqueda | O(n) — depende del dataset | O(m) — depende del patrón |
| Soporta búsqueda por subcadena | ❌ no | ✅ sí |

La búsqueda navega el árbol consumiendo el patrón carácter por carácter hasta agotarlo, luego recolecta todos los IDs del subárbol resultante. Esto significa que el tiempo de búsqueda depende únicamente de la **longitud del patrón buscado** y no del tamaño del dataset, a diferencia de la búsqueda lineal que recorrería las 34,886 películas completas en cada consulta.
> El resultado se escribe en `wiki_movie_plots_deduped_final.csv`, dejando el archivo original intacto.

## Algoritmo de Búsqueda

El algoritmo se divide en dos fases: **navegación** y **recolección**.

### Fase 1 — Navegación (`_navegar`)

Consume el patrón carácter por carácter bajando por los nodos del árbol:

```
buscar("bar")
        ↓
raiz → ¿tiene hijo que empiece con 'b'?
        ↓ sí → hijo con etiqueta "barco"
¿"barco" contiene "bar" como prefijo?
        ↓ sí → patrón agotado, llegamos al nodo
```

Hay tres casos posibles al comparar el patrón con la etiqueta del nodo:

```cpp
// caso 1 — patrón se agota dentro de la etiqueta → encontrado
if (coincide >= patron.size())
    return hijo;

// caso 2 — etiqueta se agota antes que el patrón → bajar al siguiente nodo
if (coincide < patron.size() && coincide == hijo->etiqueta.size())
    return _navegar(hijo, patron.substr(coincide));

// caso 3 — divergencia → no existe
return nullptr;
```

### Fase 2 — Recolección (`_recolectar`)

Desde el nodo donde llegó la navegación, recorre **todo el subárbol** hacia abajo recolectando todos los `movie_ids`:

```
nodo "bar"
├─ "co" → [42, 103]
│   └─ "s" → [17]
└─ "ca" → [987]

recolectar → [42, 103, 17, 987]
```

```cpp
void _recolectar(nodo, out) {
    // agrega los ids de este nodo
    for (int id : nodo->movie_ids) out.push_back(id);
    // baja recursivamente por todos los hijos
    for (auto& [_, hijo] : nodo->hijos) _recolectar(hijo, out);
}
```

### Complejidad

| Fase | Complejidad | Depende de |
|---|---|---|
| Navegación | O(m) | longitud del patrón `m` |
| Recolección | O(k) | cantidad de resultados `k` |
| Deduplicar | O(k log k) | ordenar los ids encontrados |

> El dataset de 34,886 películas **no afecta** el tiempo de navegación, solo afecta `k` que es cuántos resultados devuelve.

## Algoritmo de inserción — Árbol de Sufijos

**Entrada:** un token (ej. `"barco"`) y un `movie_id` (ej. `42`)

### Generación de sufijos

Lo primero que hace `insertar()` es generar **todos los sufijos** del token y llamar `_insertar()` para cada uno:

```
"barco"  →  "barco", "arco", "rco", "co", "o"
```

Esto es lo que convierte la estructura en un *suffix tree* real: no solo permite buscar por prefijo, sino por cualquier subcadena del token original.

---

### Los 4 casos de `_insertar()`

**Caso 1 — No existe hijo con ese carácter**
Se crea una hoja directamente.

```
árbol vacío  +  insertar("barco", 42)
────────────────────────────────────
raíz → ["barco"] {42}
```

**Caso 2 — Coincidencia exacta con la etiqueta**
El sufijo es idéntico a la etiqueta del nodo; solo se anota el ID.

```
insertar("barco", 17)  cuando ya existe  ["barco"] {42}
───────────────────────────────────────────────────────
raíz → ["barco"] {42, 17}
```

**Caso 3 — El sufijo es más corto que la etiqueta (split hacia arriba)**
El nodo existente se parte: el prefijo común sube y el resto queda como hijo.

```
existe ["barco"] {42}  +  insertar("bar", 99)
─────────────────────────────────────────────
raíz → ["bar"] {99}
            └─ ["co"] {42}
```

**Caso 3b — La etiqueta está contenida en el sufijo**
Se anota el ID en el nodo actual y se baja recursivamente con el resto del sufijo.

```
existe ["bar"] {x}  +  insertar("barco", 42)
────────────────────────────────────────────
anota 42 en ["bar"], luego desciende con "co"
```

**Caso 4 — Coincidencia parcial (split en el medio)**
Se crea un nodo intermedio con el prefijo común y dos hojas con los restos.

```
existe ["barco"] {42}  +  insertar("barca", 17)
───────────────────────────────────────────────
raíz → ["bar"] {17}
            ├─ ["co"] {42}
            └─ ["ca"] {17}
```

> El nodo `"bar"` recibe el ID `17` porque `"barca"` pasó por él, no porque `"bar"` sea una palabra completa en el catálogo.

---

### Detalles de implementación

`_anotar()` se llama en los casos 3b y 4 para evitar duplicados: revisa con `std::find` antes de hacer `push_back` en `movie_ids`.

Una vez terminadas todas las inserciones, `buscar()` navega hasta el nodo que agota el patrón y recolecta todos los IDs del subárbol hacia abajo, pasándolos por `_deduplicar()` antes de devolverlos.
