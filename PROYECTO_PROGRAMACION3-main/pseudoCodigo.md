# Integrante 4 — Estructura de Datos

## Árbol de Sufijos para Búsqueda de Películas

---

## 1. Justificación: Suffix Tree vs. Trie Estándar

Un Trie estándar indexa palabras desde su primer carácter, lo que lo limita a búsquedas por prefijo exacto. Para un motor de búsqueda de películas que debe localizar patrones dentro de sinopsis extensas — donde el término de consulta puede aparecer en el medio de una palabra o de una oración — esta restricción lo hace inviable.

Un **Suffix Tree** (árbol de sufijos comprimido o Patricia Trie) resuelve esta limitación de forma elegante: al insertar cada sufijo posible de un token, cualquier subcadena del texto original se convierte en prefijo de algún camino en el árbol. Esto garantiza búsqueda en **O(m)**, donde `m` es la longitud del patrón, independientemente del tamaño del corpus. Adicionalmente, la compresión de caminos (aristas multi-caracter) reduce drásticamente el número de nodos respecto a un Trie carácter a carácter, mejorando tanto el uso de memoria como la localidad de caché durante el recorrido.

| Característica             | Trie Estándar                      | Suffix Tree (elegido)              |
| -------------------------- | ---------------------------------- | ---------------------------------- |
| Búsqueda de substring      | No nativa — solo prefijos          | Nativa — inserta todos los sufijos |
| Búsqueda en sinopsis larga | Requiere recorrer tokens uno a uno | O(m) donde m = largo del patrón    |
| Uso de memoria             | Un nodo por carácter               | Comprimido: aristas con strings    |
| Coincidencias parciales    | Solo al inicio de la palabra       | En cualquier posición del token    |

---

## 2. Pseudo-código — Inserción

### `insertar(token, movie_id)`

Genera todos los sufijos del token y los inserta uno a uno en el árbol comprimido:

```
PARA i DESDE 0 HASTA longitud(token) - 1:
    sufijo ← token[i .. fin]
    _insertar(raíz, sufijo, movie_id)
```

### `_insertar(nodo, sufijo, movie_id)` — función interna

```
1. Si sufijo está vacío → retornar.

2. Sea primera ← sufijo[0].

3. Si nodo NO tiene hijo con clave primera:
       Crear nodo hoja con etiqueta = sufijo
       Anotar movie_id en la hoja
       Enlazar como hijos[primera] → retornar

4. Sea hijo ← nodo.hijos[primera].

5. Calcular coincide ← longitud del prefijo común entre hijo.etiqueta y sufijo.

6. Casos según coincide:
   a) coincide == longitud(sufijo) == longitud(etiqueta)
      → Anotar ID en hijo, retornar.

   b) coincide == longitud(sufijo) < longitud(etiqueta)
      → Split: crear nodo intermedio con sufijo, mover hijo debajo.

   c) coincide == longitud(etiqueta) < longitud(sufijo)
      → Anotar ID en hijo, llamar recursivamente con sufijo[coincide..].

   d) coincide < ambos
      → Split en punto de divergencia: nodo intermedio + dos ramas hijas.
```

---

## 3. Pseudo-código — Búsqueda

### `buscar(patron)` → lista de movie_ids

**FASE 1 — Navegar hasta el nodo que agota el patrón:**

```
nodo_actual ← raíz
restante    ← patron

MIENTRAS restante no esté vacío:
    primera ← restante[0]

    SI nodo_actual NO tiene hijo[primera]
        → retornar []   // no encontrado

    hijo     ← nodo_actual.hijos[primera]
    coincide ← prefijo_comun(hijo.etiqueta, restante)

    SI coincide < longitud(restante) Y coincide == longitud(etiqueta):
        nodo_actual ← hijo
        restante    ← restante[coincide..]

    SI coincide >= longitud(restante):
        nodo_actual ← hijo
        restante    ← ""        // patrón agotado

    SI coincide < longitud(restante) Y coincide < longitud(etiqueta):
        retornar []             // divergencia, no existe
```

**FASE 2 — Recolectar todos los IDs del subárbol:**

```
resultado ← []
_recolectar(nodo_actual, resultado)
deduplica y ordena resultado
retornar resultado
```

### `_recolectar(nodo, resultado)` — función interna

```
PARA cada id EN nodo.movie_ids:
    agregar id a resultado

PARA cada hijo EN nodo.hijos:
    _recolectar(hijo, resultado)
```

---

## 4. Complejidad Algorítmica

| Operación                 | Tiempo                             | Espacio                     |
| ------------------------- | ---------------------------------- | --------------------------- |
| Insertar token de largo L | O(L²) sufijos × O(L) por inserción | O(L²) nodos en el peor caso |
| Buscar patrón de largo m  | O(m)                               | —                           |
| Recolectar k resultados   | O(k)                               | O(k) en la salida           |

---

## 5. Integración con `MotorBusqueda.h`

Al cargar el CSV, construir el índice:

```cpp
SuffixTree st;
for (int i = 0; i < db.size(); i++) {
    for (auto& tok : tokenizar(db[i].titulo))   st.insertar(tok, i);
    for (auto& tok : tokenizar(db[i].sinopsis)) st.insertar(tok, i);
}
```

En la búsqueda:

```cpp
auto ids = st.buscar(limpiar(consulta));
for (int id : ids) mostrar(db[id]);
```
