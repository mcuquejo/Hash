#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "hash.h"
#include <string.h>
#include <stdio.h>


#define OCUPADO 1
#define BORRADO 2
#define VACIO 0
#define TAM_INICIAL 30
#define CARGA_MAX 0.7
#define CARGA_MIN 0.3
/* ******************************************************************
 *                           STRUCTS
 * *****************************************************************/
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
  const hash_t* hash;
  campo_t campo_act;
  size_t posicion;
};

/* ******************************************************************
 *                      FUNCIONES AUXILIARES
 * *****************************************************************/

size_t fhash(const char *s, size_t tam){
    size_t hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = (size_t)*s + 31*hashval;
    return hashval % tam;
}

bool hash_redimensionar(hash_t* hash, size_t tam){
  campo_t* campos = calloc(tam, sizeof(campo_t));
  if (campos == NULL) return false;
  for (size_t i=0; i<hash->capacidad;i++){
    if (hash->campos[i].estado != OCUPADO) continue;
    campos[fhash(hash->campos[i].clave, tam)] = hash->campos[i];
  }
  hash->capacidad = tam;
  hash->campos = campos;
  return true;
}


size_t hash_obtener_posicion(const hash_t* hash,const char* clave, int* existencia)
{
  size_t indice = fhash(clave, hash->capacidad);
  size_t pos_act = 0;
  for (size_t i=indice; i<hash->capacidad; i++){
    pos_act = (indice+i)%hash->capacidad;
    if (hash->campos[pos_act].clave == clave){
      *existencia = 1;
      return pos_act;
    }
  }
  for (size_t i=indice; i<hash->capacidad; i++){
    pos_act = (indice+i)%hash->capacidad;
    if (hash->campos[pos_act].estado != OCUPADO){
      *existencia = 0;
      return pos_act;
    }
  }
  return 0;
}



/* ******************************************************************
 *                       PRIMITIVAS DEL HASH
 * *****************************************************************/

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
   hash_t* hash = malloc(sizeof(hash_t));
   hash->cantidad = 0;
   hash->capacidad = TAM_INICIAL;
   hash->funcion_destruccion = destruir_dato;
   hash->campos = calloc(TAM_INICIAL, sizeof(campo_t));
   return hash;
 }

bool hash_guardar(hash_t *hash, const char *clave, void *dato){
   if (((double)hash->cantidad/(double)hash->capacidad) >= CARGA_MAX){
     hash_redimensionar(hash, hash->capacidad*2);
   }
   int existencia;
   size_t pos = hash_obtener_posicion(hash, clave, &existencia);

   if (existencia == 1){
     hash->campos[pos].valor = dato;
     return true;
   }
   campo_t campo;
   campo.clave = (char*)clave; //ERROR: Revisar porque clave es 'const char*' y campo.clave es 'char*'
   campo.valor = dato;
   campo.estado = OCUPADO;
   hash->campos[pos] = campo;
   hash->cantidad++;
   return true;
 }

void *hash_borrar(hash_t *hash, const char *clave){
   int existencia;
   size_t pos = hash_obtener_posicion(hash, clave, &existencia);
   void* valor = NULL;
   if (existencia == 1){
     hash->campos[pos].clave = NULL;
     hash->campos[pos].estado = BORRADO;
     valor = hash->campos[pos].valor;
     if (((double)hash->cantidad / (double)hash->capacidad) <= CARGA_MIN && hash->capacidad/2 > TAM_INICIAL){
       hash_redimensionar(hash, hash->capacidad/2);
     }
     hash->cantidad--;
   }
   return valor;
}

void *hash_obtener(const hash_t *hash, const char *clave){
   int existencia;
   size_t pos = hash_obtener_posicion(hash, clave, &existencia);
   return existencia == 1 ? hash->campos[pos].valor : NULL;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
  int existencia;
  hash_obtener_posicion(hash, clave, &existencia);
  return existencia == 1;
}

size_t hash_cantidad(const hash_t *hash){
  return hash->cantidad;
}

void hash_destruir(hash_t *hash){
  for (size_t i=0; i<hash->capacidad; i++){
    if (hash->campos[i].estado != OCUPADO) continue;
    if (hash->funcion_destruccion == NULL) break;
    hash->funcion_destruccion(hash->campos[i].valor);
  }
  free(hash->campos);
  free(hash);
}

/* ******************************************************************
 *                       PRIMITIVAS DEL ITERADOR
 * *****************************************************************/

hash_iter_t *hash_iter_crear(const hash_t *hash){
  hash_iter_t* iter = malloc(sizeof(hash_iter_t));
  if (iter == NULL) return NULL;
  iter->hash = hash;
  iter->posicion = 0;
  iter->campo_act = iter->hash->campos[iter->posicion];
  return iter;
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
  return false;
}

void hash_iter_destruir(hash_iter_t* iter){
  free(iter);
}
