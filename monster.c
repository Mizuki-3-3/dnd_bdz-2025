#include "monster.h"
Monster* create_monster(int location_id) {
    Monster *m = malloc(sizeof(Monster));
    if (!m) return NULL;
    
    switch (location_id) {
        case 1: // Часовня
            strncpy(m->name, "Оборотень-неудачник", MAX_NAME_LENGTH);
            m->level = 1;
            m->health = m->max_health = 20;
            m->strength = 8;
            m->loot_id[0] = 208; 
            m->loot_id[1] = 209;
            m->loot_id[2] = 210;
            break;
        case 4: // Круг гигантов
            strncpy(m->name, "Паук-переросток", MAX_NAME_LENGTH);
            m->level = 2;
            m->health = m->max_health = 30;
            m->strength = 12;
            m->loot_id[0] = 211;
            m->loot_id[1] = 212; 
            m->loot_id[2] = 213;
            break;
        case 6: // Лабаз
            strncpy(m->name, "Шустрый дозорный", MAX_NAME_LENGTH);
            m->level = 3;
            m->health = m->max_health = 40;
            m->strength = 15;
            m->loot_id[0] = 0; 
            m->loot_id[1] = 0; 
            m->loot_id[2] = 0;
            break;
        default:
            free(m);
            return NULL;
    }
    
    m->is_killed = 0;
    return m;
}