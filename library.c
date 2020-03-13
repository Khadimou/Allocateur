#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include "protos.h"

// recyclage de blocs: structure de données permettant d'utiliser les blocs précédemment libérés
struct free_element{
    struct free_element *next;
};

static struct free_element *free_store = NULL; //on initialise la structure de stockage d'éléments
static const int num_chunk = 4; // nombre de morceaux (nombre de blocs)

size_t nb_allocs = 0;
size_t nb_frees = 0;
char buff[1024]; //buffer temporaire de 1024 octet
unsigned int count = 0;

// initialisation des fonctions d'allocation : on définit des allocateurs statique qui vont allouer avant le dlsym renvoit son malloc
static void* (*real_malloc)(size_t size) = NULL;
static void* (*real_realloc)(void*, size_t) = NULL;
static void* (*real_calloc)(size_t, size_t) = NULL;
static void  (*real_free)(void *ptr);

// interception de la bibliothèque tierce
static void init()
{
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        real_calloc = dlsym(RTLD_NEXT, "calloc");
        real_realloc = dlsym(RTLD_NEXT,"realloc");
        real_free   = dlsym(RTLD_NEXT, "free");
        if (!real_malloc || !real_free || !real_calloc || !real_realloc )
        {
            fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
            abort();
        }
}

void *malloc(size_t size)
{
    static int initializing = 0;
    if (real_malloc == NULL)
    {
        // on réinitialise la valeur d'initialisation s'il est différent de 0 et on réintercepte malloc avec dlsym
        if (!initializing)
        {
            initializing = 1;
            init();
            initializing = 0;
        }
        // si la valeur vaut 0
        else
        {
            //si la taille allouée avec le malloc est inférieure à la taille du buffer
            if (count + size < sizeof(buff))
            {
                void *retptr = buff + count; // on définit un pointeur de taille supérieure ou égale à celle du buffer
                count += size; // on met à jour la valeur de count
                return retptr; // on retourne le pointeur
            }
            else
            {
               // si la taille allouée est superieure ou egale au buffer on exit
                exit(1);
            }
        }
    }

    void *ptr = real_malloc(size);
    nb_allocs++;
    return ptr;
}

void free(void *ptr)
{        
    if (ptr >= (void*) buff && ptr <= (void*)(buff + count)) //si le pointeur est entre le buffer et le buffer à la taille du count
        fprintf(stdout, "freeing temp memory\n"); // on affiche le message de libération de la mémoire
    else
        real_free(ptr); // sinon on appelle free de la bibliothèque tierce directement
    nb_frees++;
}

void *calloc(size_t n_elements,size_t n_octet)
{
    if (real_malloc == NULL)
    {
        void *ptr = malloc(n_elements*n_octet);
        if (ptr)
            memset(ptr, 0, n_elements*n_octet); //on remplit cette zone mémoire avec des 0
        return ptr;
    }

    void *ptr = real_calloc(n_elements, n_octet);
    nb_allocs++;
    return ptr;
}

void *realloc(void *ptr,size_t n)
{

    if (real_malloc == NULL)
    {
        void *nptr = malloc(n);
        if (nptr && ptr)
        {
            memmove(nptr, ptr, n); // on recopie n octects depuis ptr vers nptr
            free(ptr); // on libère ptr
        }
        return nptr;
    }

    void *nptr = real_realloc(ptr, n); //on modifie la taille de ptr à n octects
    nb_allocs++;
    return nptr;
}

// memory mapping
void* alloc_pages(size_t s)
{

	uint16_t *proj = mmap(NULL,taille,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
	if(proj == MAP_FAILED){
		printf("%s\n",strerror(errno));
		abort();
	}

    munmap(proj,taille);
    return proj;
}

// pool de recyclage
void *allocate(size_t size){
    struct free_element *e;
    if(!free_store){
        // la liste chaînée est vide, allocate a big chunk
        size_t big_chunk_size = num_chunk * size;
        free_store = e = (struct free_element *)(malloc(big_chunk_size));

        //traiter le big chunk comme une liste chaînée de free_element
        for(int i=0;i<num_chunk - 1;i++){
            e->next = (struct free_element*)((char*)(e) + size);
            e = e->next;
        }
        e->next = NULL; //on fait pointer le dernier petit bloc vers NULL
    }
    //return le next du free_store
    e = free_store;
    free_store = free_store->next;
        
    return e;
}


void deallocate(void *e,size_t n)
{
    //mettre le pointeur e dans le free store
    ((struct free_element*)(e))->next = free_store;
    free_store = (struct free_element*)(e);
}

// alignement

typedef uint16_t offset_t;
#define PTR_OFFSET_SZ sizeof(offset_t)

#ifndef align_up
#define align_up(num, align) \
    (((num) + ((align) - 1)) & ~((align) - 1))
#endif

void * aligned_malloc(size_t align, size_t size)
{
    void * ptr = NULL;

    // On veut que ca soit une puissance de 2 puisque
    // align_up fonctionne sur la base des puissances de 2
    assert((align & (align - 1)) == 0);

    if(align && size)
    {
        /*
         On sait qu'il faut tenir compte d'une valeur d'offset
         Nous allouons également des octets supplémentaires pour garantir que nous
         puissions répondre à l'alignement
         */
        uint32_t hdr_size = PTR_OFFSET_SZ + (align - 1);
        void * p = malloc(size + hdr_size);

        if(p)
        {
            /*
             * Ajouter la taille du décalage au pointeur du Malloc
             * (nous conserverons toujours cela)
             * Ensuite, alignez la valeur résultante sur l'alignement du cible
             */
            ptr = (void *) align_up(((uintptr_t)p + PTR_OFFSET_SZ), align);

            // on calcule l'offset et on le stocke
            // derrière notre pointeur aligné
            *((offset_t *)ptr - 1) =
                    (offset_t)((uintptr_t)ptr - (uintptr_t)p);

        } // else NULL, on peut pas appeler malloc
    } //else NULL, arguments invalides

    return ptr;
}

void aligned_free(void * ptr)
{
    assert(ptr); //on teste le pointeur

    /*
    Marchez à reculons à partir du pointeur passé pour obtenir le décalage du pointeur.
     Nous convertissons le pointeur en un pointeur offset_t
     et nous nous appuyons sur les mathématiques des pointeurs pour obtenir les données
    */
    offset_t offset = *((offset_t *)ptr - 1);

    /*
    * Maintenant qu'on a l'offset, on appelle
     le pointeur original qu'on passe à free
    */
    void * p = (void *)((uint8_t *)ptr - offset);
    free(p);
}

__attribute__((constructor))
void initialize()
{
	printf("In custom Malloc library !\n");
}

__attribute__((destructor))
void finalize()
{
	printf("Made %lu allocations\n", nb_allocs);
	printf("Made %lu frees\n", nb_frees);
}