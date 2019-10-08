#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "hash.h"
#include <string.h>
#include <stdio.h>


#define OCUPADO 1
#define BORRADO 2
#define VACIO 0
#define NO_OCUPADO 3
#define TAM_INICIAL 31
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



char* strdup (const char* s) {
	char* dup = malloc(strlen (s)+1);
	if (dup == NULL)
		return NULL;
  strcpy(dup,s);
	return dup;
}

size_t fhash(const char *s, size_t tam){
    size_t hashval;

    for (hashval = 0; *s != '\0'; s++)
        hashval = (size_t)*s + 31*hashval;
    return hashval % tam;
}

campo_t crear_campo(char* clave, void* dato, size_t estado){
  campo_t campo;
  campo.estado = estado;
  campo.clave = clave;
  campo.valor = dato;
  return campo;
}





size_t hash_buscar(const hash_t* hash,const char* clave){
  size_t indice = fhash(clave, hash->capacidad);
  size_t pos_act = 0;
  for (size_t i=0; i<hash->capacidad; i++){
    pos_act = (indice+i)%hash->capacidad;
    if (hash->campos[pos_act].estado == OCUPADO){
      if (strcmp(hash->campos[pos_act].clave,clave)==0) return pos_act;
    }

  }
  return 0;
}

size_t hash_buscar_sig(hash_t* hash,const char* clave){
  size_t indice = fhash(clave, hash->capacidad);
  size_t pos_act = 0;
  for (size_t i=0; i<hash->capacidad; i++){
    pos_act = (indice+i)%hash->capacidad;
    if (hash->campos[pos_act].estado != OCUPADO){
      return pos_act;
    }

  }
  return 0;
}



 bool campos_copiar(hash_t*hash, const char *clave, void *dato){
    size_t pos;
    if (hash_pertenece(hash, clave)){
       pos = hash_buscar(hash, clave);
       hash->campos[pos].valor = dato;
       return true;
    }
    campo_t campo = crear_campo(strdup(clave), dato, OCUPADO);
    pos = hash_buscar_sig(hash, clave);
    hash->campos[pos] = campo;
    return true;
  }

 bool hash_redimensionar(hash_t* hash, size_t tam){
   campo_t* campos_nuevo = malloc(tam * sizeof(campo_t));
   if (campos_nuevo == NULL) return false;
   campo_t* campos_act = hash->campos;
   hash->campos = campos_nuevo;
   for (size_t i=0;i<tam; i++){
     hash->campos[i] = crear_campo("", NULL, VACIO);
   }
   for (size_t i=0; i<hash->capacidad;i++){
     if (campos_act[i].estado == OCUPADO){
       campos_copiar(hash, campos_act[i].clave, campos_act[i].valor);
     }
   }
   hash->capacidad = tam;
   free(campos_act);
   return true;
 }

/* ******************************************************************
 *                       PRIMITIVAS DEL HASH
 * *****************************************************************/

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
   hash_t* hash = malloc(sizeof(hash_t));
   hash->cantidad = 0;
   hash->capacidad = TAM_INICIAL;
   hash->funcion_destruccion = destruir_dato;
   hash->campos = malloc(TAM_INICIAL*sizeof(campo_t));
   for (size_t i=0;i<TAM_INICIAL; i++){
     hash->campos[i] = crear_campo("", NULL, VACIO);
   }
   return hash;
 }




bool hash_guardar(hash_t *hash, const char *clave, void *dato){
  size_t pos;
  if (hash_pertenece(hash, clave)){
     pos = hash_buscar(hash, clave);
     hash->campos[pos].valor = dato;
     return true;
  }
  if (((double)hash->cantidad)/(double)hash->capacidad >= CARGA_MAX){

    hash_redimensionar(hash, hash->capacidad*2);
  }
  campo_t campo = crear_campo(strdup(clave), dato, OCUPADO);
  pos = hash_buscar_sig(hash, clave);
  hash->campos[pos] = campo;
  hash->cantidad++;
  return true;
}

void *hash_borrar(hash_t *hash, const char *clave){
   size_t pos;
   if (!hash_pertenece(hash, clave)) return NULL;
   pos = hash_buscar(hash, clave);
   //AGREGAR FUNCION DE DESTRUCCION.
   void* dato = hash->campos[pos].valor;
   hash->campos[pos].estado = BORRADO;
   hash->campos[pos].valor = NULL;
   hash->campos[pos].clave ="";
   if ((double)hash->cantidad/(double)hash->capacidad <= CARGA_MIN && hash->capacidad/2>TAM_INICIAL){
     hash_redimensionar(hash, hash->capacidad/2);
   }
   hash->cantidad--;
   return dato;
}

void *hash_obtener(const hash_t *hash, const char *clave){
   bool ok;
   ok = hash_pertenece(hash, clave);
   if (ok) return hash->campos[hash_buscar(hash, clave)].valor;
   return NULL;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
  size_t indice = fhash(clave, hash->capacidad);
  size_t pos_act = 0;
  if (hash->cantidad == 0) return false;
  for (size_t i=0; i<hash->capacidad; i++){
    pos_act = (indice+i)%hash->capacidad;
    if (strcmp(hash->campos[pos_act].clave,clave)==0 && hash->campos[pos_act].estado == OCUPADO){
      return true;
    }
  }
  return false;
}

size_t hash_cantidad(const hash_t *hash){
  return hash->cantidad;
}

void hash_destruir(hash_t *hash){
  for (size_t i=0; i<hash->capacidad; i++){
    if (hash->campos[i].estado != OCUPADO) continue;
    if (hash->funcion_destruccion != NULL){
      hash->funcion_destruccion(hash->campos[i].valor);
    }
  }
  free(hash->campos);
  free(hash);
}

void imprimir(const hash_t* hash){
  printf("%s\n","IMPRESION DE HASH" );
  for (size_t i=0; i<hash->capacidad; i++){
    if (hash->campos[i].estado==OCUPADO){
      printf("%s\n", hash->campos[i].clave);
    }
    if (hash->campos[i].estado == VACIO){
      printf("%s\n", "VACIO");
    }
    if (hash->campos[i].estado == BORRADO){
      printf("%s\n", "BORRADO");
    }
  }
}

/* ******************************************************************
 *                       PRIMITIVAS DEL ITERADOR
 * *****************************************************************/

hash_iter_t *hash_iter_crear(const hash_t *hash){
  hash_iter_t* iter = malloc(sizeof(hash_iter_t));
  if (iter == NULL) return NULL;
  iter->hash = hash;
  if (iter->hash->cantidad ==0){
    iter->posicion = 0;
    iter->campo_act = iter->hash->campos[0];
    return iter;
  }
  iter->posicion = 0;
  while (iter->hash->campos[iter->posicion].estado != OCUPADO){
    iter->posicion++;
  }
  iter->campo_act = iter->hash->campos[iter->posicion];
  return iter;
}

bool hash_iter_avanzar(hash_iter_t *iter){
  if (hash_iter_al_final(iter)) return false;
  iter->posicion++;
  iter->campo_act = iter->hash->campos[iter->posicion];
  if (iter->campo_act.estado != OCUPADO) hash_iter_avanzar(iter);
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
