

typedef struct hash hash_t;
typedef struct hash_iter hash_iter_t;
typedef void (*hash_destruir_dato_t)(void *);

hash_t *hash_crear(hash_destruir_dato_t destruir_dato);
bool hash_guardar(hash_t *hash, const char *clave, void *dato);
void *hash_borrar(hash_t *hash, const char *clave);
void *hash_obtener(const hash_t *hash, const char *clave);
bool hash_pertenece(const hash_t *hash, const char *clave);
size_t hash_cantidad(const hash_t *hash);
void hash_destruir(hash_t *hash);


hash_iter_t *hash_iter_crear(const hash_t *hash);
bool hash_iter_avanzar(hash_iter_t *iter);
const char *hash_iter_ver_actual(const hash_iter_t *iter);
bool hash_iter_al_final(const hash_iter_t *iter);
void hash_iter_destruir(hash_iter_t* iter);
