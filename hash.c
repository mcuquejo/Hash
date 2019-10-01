
#define OCUPADO 1
#define BORRADO -1
#define VACIO 0
#define TAM_INICIAL 30

typedef struct campo{
  char* clave;
  void* valor;
  size_t estado;
}campo_t;

struct hash{
  size_t capacidad;
  size_t cantidad;
  hash_destruir_dato_t funcion_destruccion;
  campo_t* campos;
};

struct hash_iter{
  hash_t hash;
  campo_t campo_act;
  size_t posicion;
};
typedef void (*hash_destruir_dato_t)(void *);

int hash_obtener_posicion(hash_t* hash, char* clave, int* existencia){
  int indice = fhash(clave);
  int pos_act = 0;
  for (int i=indice; i<hash->capacidad; i++){
    pos_act = (indice+i)%hash->capacidad;
    if (hash->campos[pos_act].clave == clave){
      *existencia = 1;
      return pos_act;
    }
    if (hash->campos[pos_act].estado == BORRADO) continue;
    if (hash->campos[pos_act].estado == VACIO){
      *existencia = 0;
      return 0;
    }
  }
  *existencia = 0;
  return 0;
}

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
  hash_t* hash = malloc(sizeof(hash_t));
  hash->cantidad = 0;
  hash->capacidad = TAM_INICIAL;
  hash->funcion_destruccion = destruir_dato;
  hash->campos = calloc(TAM_INICIAL, sizeof(campo_t));
  return hash;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
  int existencia;
  int pos = hash_obtener_posicion(hash, clave, &existencia);
  campo_t campo;
  campo.clave = clave;
  campo.valor = dato;
  if (existencia == 1){
    hash->campos[pos].valor = dato;
    return true;
  }
  bool pertenencia = hash_pertenece(hash, clave);
  for (int i = fhash(clave); i<hash->capacidad; i++){
    if (hash->campos[i].clave == clave){
      hash->campos[i] = campo;
      return true;
    }
    if (!pertenencia && hash->campos[i].estado != OCUPADO){
      hash->campos[i] = campo;
      return true;
    }
  }
  for (int i = 0; i<fhash(clave); i++){
    if (hash->campos[i].clave == clave){
      hash->campos[i] = campo;
      return true;
    }
    if (!pertenencia && hash->campos[i].estado != OCUPADO){
      hash->campos[i] = campo;
      return true;
    }
  }
  return false;
}

void *hash_borrar(hash_t *hash, const char *clave){
  int existencia;
  int pos = hash_obtener_posicion(hash, clave, &existencia);
  if (existencia == 1){
    hash->campos[pos].estado = BORRADO;
    return hash->campos[pos].valor;
  }
}

void *hash_obtener(const hash_t *hash, const char *clave){
  int existencia;
  int pos = hash_obtener_posicion(hash, clave, &existencia);
  return existencia == 1 ? hash->campos[pos].valor : NULL;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
  int existencia;
  int pos = hash_obtener_posicion(hash, clave, &existencia);
  return *existencia == 1;
}

size_t hash_cantidad(const hash_t *hash){
  return hash->cantidad;
}

void hash_destruir(hash_t *hash);


hash_iter_t *hash_iter_crear(const hash_t *hash){
  hash_iter_t* iter = malloc(sizeof(hash_iter_t));
  if (iter == NULL) return NULL;
  iter->hash = hash;
  iter->posicion = 0;
  iter->campo_act = iter->hash->campos[iter->posicion];
}

bool hash_iter_avanzar(hash_iter_t *iter){
  if (hash_iter_al_final(iter)) return false;
  iter->posicion++;
  iter->campo_act = iter->hash->campos[iter->posicion];
  return true;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
  if (hash_iter_al_final(iter)) return NULL;
  if (iter->hash->cantidad == 0) return NULL;
  return iter->campo_act.clave;
}
bool hash_iter_al_final(const hash_iter_t *iter){
  if (iter->hash->cantidad == 0) return true;
  if (iter->hash->capacidad == iter->posicion) return true;

}
void hash_iter_destruir(hash_iter_t* iter);
