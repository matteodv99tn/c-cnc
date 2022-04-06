
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ================================================================ //
// ============ STRUCTURES ======================================== //
// ================================================================ //

// Structure representing an object in the list
typedef struct element{ 

  char    *id;

  struct element    *prev;  // We cannot say element_t *prev because element_t is not yet defined!
  struct element    *next;

} element_t;

// Structure representing the list itself
typedef struct{

  element_t *first;   // pointer to first element of the list
  element_t *last;    // pointer to last element of the list

  size_t length;      // size of the list

} list_t;

typedef enum { 
  ASC,
  DESC
} loop_order_t;

typedef void (*loop_fun_t)(element_t *e, loop_order_t order, void *userdata) loop_fun_t;

// ================================================================ //
// ============ FUNCTION DECLARATION ============================== //
// ================================================================ //

list_t * list_new(char *id);

// Appends  an existing element to the list
void list_append_element(list_t *list, element_t *e);

// Create and appends an element
void list_append(list_t *list, char *id);

// Inserts an existing element
void list_insert_element(list_t *list, element_t *new, char *after);

// Create and inserts the element
void list_insert(list_t *list, char *id, char *after);

// Deletes an entry with a specified id
void list_delete(list_t *list, char *id);

// Destroys the list
void list_free(list_t *list);

void list_loop(list_t *list, loop_fun_t fun, loop_order_t order, void *userdata);

// ================================================================ //
// ============ ENTRY POINT ======================================= //
// ================================================================ //
int main(){

  int *pt = NULL;

  if(pt) printf("true\n");
  else printf("false\n");

  return 0;
}


// ================================================================ //
// ============ IMPLEMENTATION OF FUNCTIONS ======================= //
// ================================================================ //

list_t * list_new(char *id){

  list_t *l = malloc(sizeof(list_t));
  memset(l, 0, sizeof(list_t));   // set to zero all the bytes

  // create a new element
  element_t *e = malloc(sizeof(element_t));
  memset(e, 0, sizeof(element_t));

  e->id = malloc(strlen(id) + 1);
  strncpy(e->id, id, strlen(id));

  e->next   = NULL;
  e->prev   = NULL;

  l->first  = e;
  l->last   = e;
  l->length = 1;

  /*
   * Note: in this case memset is useless because we assign values by hands
   * but it's usuful if nextly we add new values to element_t/list_t
   * in order to have a pre-defined value while not specify it!
   */
  
  return l;

}

void list_append_element(list_t *list, element_t *e){

  list->last->next = e;
  e->prev       = list->last;
  list->last    = e;
  e->next       = NULL;

  list->length++;
}

void list_append_(list_t *list, char *id){

  element_t *e = malloc(sizeof(element_t));
  memset(e, 0, sizeof(element_t));
  
  e->id = malloc(strlen(id) + 1);
  strncpy(e->id, id, strlen(id));

  list_append_element(list, e);

}

void list_insert_element(list_t *list, element_t *new, char *after){

  element_t *curr = list->first;

  while( strcmp(curr->id, after) != 0 && curr != NULL )
    curr = curr->next;

  if(curr != NULL){

    // if the element with label after has been encountered
    curr->next->prev  = new;
    new->next         = curr->prev;
    new->prev         = curr;
    curr->next        = new;

  } else {

    // if the element with label after has not been encoutered, append at the end
    list->last->next  = new;
    new->prev         = list->last;
    list->last        = new;
    new->next         = NULL;

  }

  list->length++;

}

void list_insert(list_t *list, char *id, char *after){

  element_t *e = malloc(sizeof(element_t));
  memset(e, 0, sizeof(element_t));

  e->id = malloc( strlen(id) + 1 );
  strncpy(e->id, id, strlen(id));

  list_insert_element(list, e, after);

}

void list_delete(list_t *list, char *id){

  element_t *curr = list->first;
  
  while( strcmp(curr->id, id) != 0 && curr != NULL )
    curr = curr->next;

  if(curr != NULL){

    curr->next->prev  = curr->prev;
    curr->prev->next  = curr->next;

    free(curr->id);
    free(curr);

    list->length--;

  }

}

void list_free(list_t* list){

  element_t *curr = list->first;
  element_t *next;

  while(curr != NULL){

    next = curr->next;
    
    free(curr->id);
    free(curr);

    curr = next;

  }

  free(list);

}

void list_loop(list_t *list, loop_fun_t fun, loop_order_t order, void *userdata){

  element_t *e;

  if(order == ASC) e = list->first;
  else             e = list->last;

  while(e != NULL){
    
    fun(e, order, userdata);

    if(order == ASC) e = e->next;
    else             e = e->prev;

  }

}




