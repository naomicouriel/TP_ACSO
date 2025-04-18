#define _POSIX_C_SOURCE 200809L

#include "ej1.h"
#include <string.h>

/**
 * Crea una nueva lista vacía.
 */
string_proc_list* string_proc_list_create(void){
	string_proc_list* list = (string_proc_list*)malloc(sizeof(string_proc_list));
    if (!list) return NULL;
    list->first = NULL;
    list->last = NULL;
    return list;
}

/**
 * Crea un nuevo nodo con el tipo y hash proporcionado.
 * El hash no se copia, solo se apunta al mismo.
 */
string_proc_node* string_proc_node_create(uint8_t type, char* hash){
    string_proc_node* node = malloc(sizeof(string_proc_node));
    if (node == NULL) return NULL;

    node->next = NULL;
    node->previous = NULL;
    node->hash = hash;  // solo apuntar, no copiar
    node->type = type;

    return node;
}

/**
 * Agrega un nodo nuevo al final de la lista con el tipo y hash dado.
 * El hash no se copia, solo se apunta al mismo.
 */
void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash){
	string_proc_node* new_node = string_proc_node_create(type, hash);
	if (!new_node) return;

	if (list->first == NULL) {
		list->first = new_node;
		list->last = new_node;
	} else {
		new_node->previous = list->last;
		list->last->next = new_node;
		list->last = new_node;
	}
}

/**
 * Concatena todos los hashes de nodos cuyo tipo coincide con el pasado,
 * empezando por el hash dado como parámetro.
 * Retorna el nuevo string concatenado (malloc).
 */
char* string_proc_list_concat(string_proc_list* list, uint8_t type , char* hash){
	char* result = strdup(hash); // Crear copia inicial de hash
    if (!result) return NULL;

    string_proc_node* current = list->first;
    while (current != NULL) {
        if (current->type == type) {
            char* new_result = str_concat(result, current->hash);
            free(result);
            result = new_result;
        }
        current = current->next;
    }

    return result;
}


/** AUX FUNCTIONS **/

void string_proc_list_destroy(string_proc_list* list){
	if (list == NULL) {
        return; // Si la lista es NULL, no hacemos nada
    }
	/* borro los nodos: */
	string_proc_node* current_node	= list->first;
	string_proc_node* next_node		= NULL;
	while(current_node != NULL){
		next_node = current_node->next;
		string_proc_node_destroy(current_node);
		current_node	= next_node;
	}
	/*borro la lista:*/
	list->first = NULL;
	list->last  = NULL;
	free(list);
}
void string_proc_node_destroy(string_proc_node* node){
    if (node == NULL) {
        return;
    }
    node->next      = NULL;
    node->previous  = NULL;
    // Remove: node->hash = NULL;
    node->type      = 0;            
    free(node);
}

char* str_concat(char* a, char* b) {
	int len1 = strlen(a);
    int len2 = strlen(b);
	int totalLength = len1 + len2;
    char *result = (char *)malloc(totalLength + 1); 
    strcpy(result, a);
    strcat(result, b);
    return result;  
}

void string_proc_list_print(string_proc_list* list, FILE* file){
        uint32_t length = 0;
        string_proc_node* current_node  = list->first;
        while(current_node != NULL){
                length++;
                current_node = current_node->next;
        }
        fprintf( file, "List length: %d\n", length );
		current_node    = list->first;
        while(current_node != NULL){
                fprintf(file, "\tnode hash: %s | type: %d\n", current_node->hash, current_node->type);
                current_node = current_node->next;
        }
}