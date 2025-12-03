#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hero.h"

Hero* create_hero(const char* name, int class) {
    if (!name || strlen(name) == 0) return NULL;

    Hero* h = malloc(sizeof(Hero));
    if (!h) return NULL;
    switch (class)
    {
    case 1:
        h->level = 1;
        h->hp = 20;
        h->mp = 35;
        h->strength = 10;
        h->dexterity = 10;
        h->magic = 20;
        h->exp = 0;
        h->exp_to_next = 50;
        break;
    case 2:
        h->level = 1;
        h->hp = 35;
        h->mp = 20;
        h->strength = 20;
        h->dexterity = 10;
        h->magic = 10;
        h->exp = 0;
        h->exp_to_next = 50;
        break;
    case 3:
        h->level = 1;
        h->hp = 27;
        h->mp = 28;
        h->strength = 10;
        h->dexterity = 20;
        h->magic = 10;
        h->exp = 0;
        h->exp_to_next = 50;
        break;
    
    default:
        break;
    }




}