
#include <stdio.h>
#include <stdlib.h>
#include "bag.h"

int main()
{
  bag_t *bag;
  bag = bag_new();

  bag_insert(bag, "poop");
  bag_insert(bag, "poopily");

  //  bag_delete(bag);

  return 0;

}
