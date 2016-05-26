/*
 * bag.c - data structure that stores void pointers
 *
 * Code for bag.c based on tree.c from Professor Kotz's tree6 module
 *
 * Drew Waterman, April 2016
 */

#include <stdio.h>
#include <stdlib.h>

/**************** local types ****************/
typedef struct bagnode {
  
  // data that corresponds to the item in the bag
  void* data;
  
  // next item in the linked list that makes up bag structure 
  struct bagnode *next;
  
} bagnode_t;


/**************** global types ****************/
typedef struct bag {
  bagnode_t *head; // head of linked list
  //  void (*itemdelete)(void *data);
} bag_t;

/**************** global functions ****************/
/* see bag.h for comments about exported functions */


/**************** local functions ****************/
/* not visible outside this file */
static bagnode_t *bagnode_new(void *data);


/**************** bag_new() ****************/
bag_t
*bag_new()
{
  // allocate memory
  bag_t *bag = malloc(sizeof(bag_t));
  
  // return NULL if there was a problem allocating memory
  if (bag == NULL) {
    return NULL;
  } else {
    // set the head to NULL initially and return the new bag
    bag->head = NULL;
    //    bag->itemdelete = itemdelete;
    return bag;
  }
}

/**************** bagnode_new ****************/
static bagnode_t *
bagnode_new(void *data) {
  // allocate space for a new node
  bagnode_t *node = malloc(sizeof(bagnode_t));

  // return NULL if there were problems
  if (node == NULL)
    return NULL;
  else {
    // otherwise set its data to the data given as a parameter  
    node->data = data;
    // set the next pointer to NULL because there's nothing for
    // it to point at yet
    node->next = NULL;

    // return the node we created
    return node;
  }
}

/**************** bag_insert() ****************/
void
bag_insert(bag_t *bag, void *data)
{
  // check to make sure that the bag exists
  if (bag != NULL) {

    // create a new node that holds the data
    bagnode_t *newnode = bagnode_new(data);

    // if the head is NULL, there are no items in the bag yet
    // set head to be the node we just created
    if (bag->head == NULL) { 
      bag->head = newnode;
    }

    // otherwise, the bag is not empty
    else {
      bagnode_t *current = bag->head;
      // loop through until we reach the end of the list 
      while (current->next != NULL) {
	current = current->next;
      }
      // put the new node at the end of the list
      current->next = newnode;
    }
  }
}

/**************** bag_extract() ****************/
void *
bag_extract(bag_t *bag)
{
  if (bag != NULL) {

    // "current" keeps track of which nose we are accessing
    bagnode_t *current = bag->head;
    if (current == NULL)
      return NULL; // the list is empty

    // keep track of the node before the "current" node
    bagnode_t *currentParent = current;

    // loop through the list until we get to the end, starting at the head     
    while (current->next != NULL) {
      currentParent = current;
      current = current->next;
    }

    // store value of data at the last node in a temporary variable
    void *currentData = current->data;
    /* if (bag->itemdelete != NULL)  */
    /*   (*bag->itemdelete)(current->data); */
    
    // check to see if the item we want to extract is the head of the list
    // if it is the head, free it and set the head to NULL    
    if (current == bag->head) {
      free(current);
      bag->head = NULL;
    }
    
    // free the memory previously allocated to the node we are extracting
    free(currentParent->next);

    // set the "next" pointer of the node before it to NULL
    currentParent->next = NULL;
    
    return currentData; // return the data that we temporarily stored
  }
  return NULL; // return NULL if the bag didn't exist
}



/* void */
/* bag_delete(bag_t *bag) */
/* { */
/*   if (bag != NULL) { */
/*     if (bag->head != NULL) { */

/*       bagnode_t *current = bag->head; */
/*       while (current != NULL) { */
/*         bagnode_t *next = current->next; */
/* 	if (bag->itemdelete != NULL) { */
/* 	  (*bag->itemdelete)(current->data); */
/* 	} */
/* 	free(current); */
/* 	current = next; */
/*       } */
/*     } */
/*     free(bag); */
/*   } */
/* } */
