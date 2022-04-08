
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

typedef void (*loop_fun_t)(element_t *e, loop_order_t o, void *userdata);

// ================================================================ //
// ============ FUNCTION DECLARATION ============================== //
// ================================================================ //

list_t * list_new(char *id);

// Appends  an existing element to the list
void list_append_element(list_t *list, element_t *e);

// Create and appends an element
element_t * list_append(list_t *list, char *id);

// Inserts an existing element
void list_insert_element(list_t *list, element_t *new, char *after);

// Create and inserts the element
element_t * list_insert(list_t *list, char *id, char *after);

// Deletes an entry with a specified id
void list_delete(list_t *list, char *id);

// Destroys the list
void list_free(list_t *list);

void list_loop(list_t *list, loop_fun_t fun, loop_order_t order, void *userdata);

element_t * element_new(char *id);

void print_element_with_index(element_t *e, loop_order_t o, void *userdata){
  size_t *i = (size_t *)userdata;
  printf("[%lu] %s\n", *i, e->id);
  if(o == ASC)    
    (*i)++;
  else 
    (*i)--;
}

void print_element(element_t *e, loop_order_t o, void *userdata){
  printf("%10s: %15p -> %15p -> %15p\n", e->id, e->prev, e, e->next);
}

// ================================================================ //
// ============ ENTRY POINT ======================================= //
// ================================================================ //
int main(){

  char      id[4][10] = {"one", "two", "three", "four"};
  element_t *e;
  size_t    i;
  
  // Create a list
  list_t *list = list_new("zero");

  // Populate the list with other elements
  for(i = 0; i < 4; i++){

    // Create an element and append it to the list
    e = element_new(id[i]);
    list_append_element(list, e);

    // Or
    // e = list_append(list, id[i]);
  }


  // Access the element of the list
  list_loop(list, print_element, ASC, NULL);  

  list_insert(list, "two.five", "two");
  i = 0;
  list_loop(list, print_element_with_index, ASC, &i);

  // Free the list
  list_free(list);

  return 0;
}   

// ================================================================ //
// ============ IMPLEMENTATION OF FUNCTIONS ======================= //
// ================================================================ //
#pragma region implementation of the declared functions

element_t * element_new(char *id){

  element_t *e = malloc(sizeof(element_t));
  memset(e, 0, sizeof(element_t));
  e->id = malloc(strlen(id) + 1);
  strncpy(e->id, id, strlen(id));

  return e;
}

void element_free(element_t *e){
  free(e->id);
  free(e);
}

list_t * list_new(char *id){

  list_t *l = malloc(sizeof(list_t));
  memset(l, 0, sizeof(list_t));   // set to zero all the bytes

  // create a new element
  element_t *e = element_new(id);

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

element_t * list_append(list_t *list, char *id){

  element_t *e = element_new(id);
  list_append_element(list, e);

  return e;

}

void list_insert_element(list_t *list, element_t *new, char *after){

  element_t *curr = list->first;

  while( strcmp(curr->id, after) != 0 && curr != NULL )
    curr = curr->next;

  if(curr != NULL){

    // if the element with label after has been encountered
    curr->next->prev  = new;
    new->next         = curr->next;
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

element_t * list_insert(list_t *list, char *id, char *after){

  element_t *e = element_new(id);
  list_insert_element(list, e, after);
  
  return e;

}

void list_delete(list_t *list, char *id){

  element_t *curr = list->first;
  
  while( strcmp(curr->id, id) != 0 && curr != NULL )
    curr = curr->next;

  if(curr != NULL){

    curr->next->prev  = curr->prev;
    curr->prev->next  = curr->next;

    element_free(curr);

    list->length--;

  }

}

void list_free(list_t* list){

  element_t *curr = list->first;
  element_t *next;

  while(curr != NULL){

    next = curr->next;
    element_free(curr);
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

#pragma endregion



