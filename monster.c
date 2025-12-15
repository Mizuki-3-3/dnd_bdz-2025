#include "monster.h"
Monster* create_monster(int location_id) {
    Monster *m = malloc(sizeof(Monster));
    if (!m) return NULL;
    
    switch (location_id) {
        case 1: // Часовня
            strncpy(m->name, "Оборотень-неудачник", MAX_NAME_LENGTH-1);
            m->level = 1;
            m->health = 15;
            m->strength = 4;
            m->loot_id[0] = 201; // Кинжал
            m->loot_id[1] = 103; // Аптечка
            m->loot_id[2] = 0;
            break;
        case 4: // Круг гигантов
            strncpy(m->name, "Паук-переросток", MAX_NAME_LENGTH-1);
            m->level = 2;
            m->health = 20;
            m->strength = 6;
            m->loot_id[0] = 202; // Сапоги
            m->loot_id[1] = 103; // Аптечка
            m->loot_id[2] = 0;
            break;
        case 6: // Лабаз
            strncpy(m->name, "Шустрый дозорный", MAX_NAME_LENGTH-1);
            m->level = 3;
            m->health = 30;
            m->strength = 8;
            m->loot_id[0] = 203; // Рубаха
            m->loot_id[1] = 103; // Аптечка
            m->loot_id[2] = 0;
            break;
    }
    
    m->is_killed = 0;
    return m;
}