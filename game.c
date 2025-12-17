#include "interface.h"
#include "history.h"
#include "hero.h"
#include "inventory.h"
#include "database.h"
#include "monster.h"
#include "location.h"
#include "combat.h"
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>

curw *narrative_win = NULL;  
curw *inventory_win = NULL;
item_database global_db;
Hero *player = NULL;
inventory *player_inv = NULL;
game_state state;
int current_location = 0;
int prev_location = 0;
char player_name[MAX_NAME_LENGTH];
int chosen_class;

static int location_connections[7][7] = {// Матрица смежности локаций 
    {0, 1, 1, 0, 0, 0, 0},  // 0 - Берег реки
    {1, 0, 0, 1, 0, 0, 0},  // 1 - Часовня
    {1, 0, 0, 1, 0, 0, 0},  // 2 - Пещера масок
    {0, 1, 1, 0, 1, 1, 0},  // 3 - Указатель
    {0, 0, 0, 1, 0, 1, 0},  // 4 - Круг гигантов
    {0, 0, 0, 1, 1, 0, 1},  // 5 - Родник
    {0, 0, 0, 0, 0, 1, 0}   // 6 - Лабаз
};

static const char* location_names[7] = {
    "Берег бурной реки",
    "Лесная часовенка",
    "Пещера молчаливых масок",
    "Старый указатель",
    "Круг поваленных гигантов",
    "Холодный родник",
    "Лабаз в корнях"
};

static void show_current_location(item_template *loc);
static void take_treasure(int location_id);
static void start_combat(int location_id);
static void player_attack(Monster* monster, int use_magic);
static void monster_attack_player(Monster* monster);
static void end_combat(int victory);
static void open_inventory(void);
static void use_item_from_inventory(int index);
static void drop_item_from_inventory(int index);
static void game_over(void);
static void victory_screen(void);
static void check_level_up(void);
static void move_to_location(int new_location);
static void create_inventory_window(void);
static void destroy_inventory_window(void);
static void open_inventory_in_combat(void);
static void give_monster_loot(Monster* monster);
static void restart_game(void);  // ← НОВАЯ ФУНКЦИЯ

void create_game_windows(void) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    narrative_win = make_new_win(1, 1, max_y - 2, max_x - 2, "==Dungeon==");
    
    inventory_win = NULL;
    
    if (player_inv) {
        player_inv->win = NULL;
    }
}

int choose_class(WINDOW *win) {
    if (!win) return 1;
    
    werase(win);
    
    mvwprintw(win, 1, 2, "Выберите класс персонажа:");
    mvwprintw(win, 3, 2, "1. Маг");
    mvwprintw(win, 4, 2, "   - Высокий урон магией");
    mvwprintw(win, 5, 2, "   - Меньше здоровья");
    
    mvwprintw(win, 7, 2, "2. Воин");
    mvwprintw(win, 8, 2, "   - Высокое здоровье и сила");
    mvwprintw(win, 9, 2, "   - Меньше маны");
    
    mvwprintw(win, 11, 2, "3. Плут");
    mvwprintw(win, 12, 2, "   - Высокая ловкость");
    mvwprintw(win, 13, 2, "   - Критические удары");
    
    mvwprintw(win, 15, 2, "Выбор (1-3): ");
    wrefresh(win);
    
    int choice;
    while (1) {
        choice = getch() - '0';
        
        if (choice >= 1 && choice <= 3) {
            break;
        } else {
            mvwprintw(win, 16, 2, "Неверный выбор! Введите 1, 2 или 3");
            mvwprintw(win, 15, 14, "   ");
            wmove(win, 15, 14);
            wrefresh(win);
        }
    }
    
    werase(win);
    wrefresh(win);
    
    return choice;
}

void init_new_game(const char *name, int class) {
    player = create_hero(name, class);
    if (!player) return;
    
    player_inv = create_inventory();
    if (!player_inv) {
        free(player);
        return;
    }
    
    strncpy(player->name, name, MAX_NAME_LENGTH - 1);
    player->name[MAX_NAME_LENGTH - 1] = '\0';
    
    inventory_add_item_by_id(player_inv, &global_db, 201, 1);
    inventory_add_item_by_id(player_inv, &global_db, 202, 1);
    inventory_add_item_by_id(player_inv, &global_db, 203, 1);
    inventory_add_item_by_id(player_inv, &global_db, 204, 1);

    current_location = 0;
    prev_location = 0;
    
    player->current_location = current_location;
    player->prev_location = prev_location;
}

int load_game(const char *name) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.save", name);
    
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;
    
    Hero temp_hero;
    size_t read = fread(&temp_hero, sizeof(Hero), 1, f);
    fclose(f);
    
    if (read != 1) return 0;
    
    player = malloc(sizeof(Hero));
    if (!player) return 0;
    memcpy(player, &temp_hero, sizeof(Hero));
    
    player_inv = create_inventory();
    
    current_location = player->current_location;
    prev_location = player->prev_location;
    
    return 1;
}

void show_current_location(item_template *loc) {
    if (!narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    werase(win);
    
    if (loc) {
        int max_y = getmaxy(win), max_x = getmaxx(win), line = 0, col = 0;
        const char *description = loc->template.location_template.description;
        print_wrapped_text(win, description);
        getyx(win, line, col);
        print_hint(win, line+2, max_y, loc);
        }
}


void take_treasure(int location_id) {
    item_template *loc = itemdb_find_by_id(&global_db, 400 + location_id);
    if (!loc || loc->template.location_template.type != LOC_TREASURE) return;
    
    int treasure_id = 0;
    switch (location_id) {
        case 2:
            treasure_id = 207;
            treasure_id = 207;
            treasure_id = 207;
            break;
        case 5:
            treasure_id = 103;
            break;
        default:
            treasure_id = 103;
    }
    
    if (inventory_add_item_by_id(player_inv, &global_db, treasure_id, 1)) {
        item_template *treasure = itemdb_find_by_id(&global_db, treasure_id);
        if (treasure && narrative_win && narrative_win->overlay) {
            WINDOW *win = narrative_win->overlay;
            werase(win);
            mvwprintw(win, 1, 2, "Вы нашли: %s", treasure->name);
            mvwprintw(win, 2, 2, "%s", treasure->description);
            mvwprintw(win, 4, 2, "Нажмите любую клавишу...");
            wrefresh(win);
            getch();
        }
    }
    loc->template.location_template.type = LOC_EMPTY;
}

void game_loop(void) {
    int load_result = 0;
    char save_filename[256];
    srand(time(NULL));
    itemdb_init(&global_db);
    init_default_items(&global_db);
    
    create_game_windows();
    
    // внешний цикл для перезапуска игры
    while (1) {
        echo();
        memset(player_name, 0, sizeof(player_name));
        
        if (narrative_win && narrative_win->overlay) {
            werase(narrative_win->overlay);
            mvwprintw(narrative_win->overlay, 1, 2, "Введите имя героя:");
            mvwprintw(narrative_win->overlay, 2, 2, "> ");
            wrefresh(narrative_win->overlay);
            wgetnstr(narrative_win->overlay, player_name, MAX_NAME_LENGTH-1);
        }
        noecho();

        
        snprintf(save_filename, sizeof(save_filename), "%s.save", player_name);

        FILE *save_file = fopen(save_filename, "rb");
        if (save_file) {
            fclose(save_file);
            
            if (narrative_win && narrative_win->overlay) {
                WINDOW *win = narrative_win->overlay;
                werase(win);
                mvwprintw(win, 1, 2, "Найдено сохранение для %s", player_name);
                mvwprintw(win, 2, 2, "Загрузить? (Y/N)");
                wrefresh(win);
                
                int ch = getch();
                if (ch == 'y' || ch == 'Y') {
                    load_result = load_game(player_name);
                } else {
                    load_result = 0;
                }
            }
        }
        
        // сброс состояния игры
        state.current_mode = MODE_NARRATIVE;
        state.previous_mode = MODE_NARRATIVE;
        state.inventory_selected_index = 0;
        state.quit_flag = 0;
        state.restart_flag = 0;
        
        if (!load_result) {
            if (narrative_win && narrative_win->overlay) {
                chosen_class = choose_class(narrative_win->overlay);
                init_new_game(player_name, chosen_class);
                chapter1(narrative_win->overlay);
            }
        }

        update_panels();
        doupdate();
        
        item_template *loc = itemdb_find_by_id(&global_db, 400);
        
        if (loc) {
            show_current_location(loc);
        }
        
        // главный игровой цикл
        while (!state.quit_flag && !state.restart_flag) {
            switch (state.current_mode) {
                case MODE_NARRATIVE:
                    loc = itemdb_find_by_id(&global_db, 400+current_location);
                    show_current_location(loc);
                    
                    int ch = getch();
                    ch = tolower(ch);
                    
                    switch (ch) {
                        case 'q':
                            state.quit_flag = 1;
                            break;
                            
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6': {
                            int new_loc = ch - '0';
                            move_to_location(new_loc);
                            break;
                        }
                            
                        case 'a':
                            if (current_location == 1 || current_location == 4 || current_location == 6) {
                                start_combat(current_location);
                            }
                            break;
                            
                        case 't':
                            if (current_location == 2 || current_location == 5) {
                                take_treasure(current_location);
                            }
                            break;
                            
                        case 'r':
                            if (prev_location >= 0 && prev_location < 7) {
                                move_to_location(prev_location);
                            }
                            break;
                            
                        case 'i':
                            open_inventory();
                            break;
                    }
                    break;
                    
                case MODE_COMBAT:
                    break;
            }
            
            update_panels();
            doupdate();
        }
        
        // очистка перед возможным перезапуском
        if (player) {
            save_hero(player, player_name);
            destroy_hero(player);
            player = NULL;
        }
        if (player_inv) {
            free_inventory(player_inv);
            player_inv = NULL;
        }
        
        if (inventory_win) {
            destroy_inventory_window();
        }
        
        // ксли нужно перезапустить - продолжаем внешний цикл
        if (state.restart_flag) {
            // сброс состояния локаций
            for (int i = 1; i < MAX_LOCATIONS; i++) {
                item_template *loc_item = itemdb_find_by_id(&global_db, 400 + i);
                if (loc_item) {
                    loc_item->template.location_template.type = 
                        loc_item->template.location_template.original_type;
                }
            }
            continue; // начинаем новую игру
        }
        
        // выход из игры
        break;
    }
    
    endwin();
}

int roll_dice(void) {
    return (rand() % 6) + 1;
}

int calculate_damage(int base, int dice_roll) {
    return base + dice_roll;
}

void check_level_up(void) {
    if (!player || !narrative_win || !narrative_win->overlay) return;
    
    if (player->exp >= player->exp_to_next) {
        player->level++;
        player->exp -= player->exp_to_next;
        player->exp_to_next = (int)(player->exp_to_next * 1.5);
        
        player->max_hp += 10;
        player->hp = player->max_hp;
        player->max_mp += 5;
        player->mp = player->max_mp;
        
        player->base_strength += 2;
        player->base_dexterity += 1;
        player->base_magic += 1;
        
        player->strength = player->base_strength;
        player->dexterity = player->base_dexterity;
        player->magic = player->base_magic;
        
        inventory_update_capacity(player_inv, player->level);
        
        WINDOW *win = narrative_win->overlay;
        werase(win);
        mvwprintw(win, 1, 2, "=== ПОВЫШЕНИЕ УРОВНЯ! ===");
        mvwprintw(win, 2, 2, "Вы достигли уровня %d!", player->level);
        mvwprintw(win, 3, 2, "Здоровье увеличено!");
        mvwprintw(win, 4, 2, "Характеристики улучшены!");
        mvwprintw(win, 5, 2, "Нажмите любую клавишу...");
        wrefresh(win);
        getch();
    }
}

void move_to_location(int new_location) {
    if (new_location < 0 || new_location >= 7) return;
    
    if (!location_connections[current_location][new_location]) {
        return;
    }
    
    prev_location = current_location;
    current_location = new_location;
    
    if (player) {
        player->current_location = current_location;
        player->prev_location = prev_location;
    }
}


void start_combat(int location_id) {
    
    Monster *monster = create_monster(location_id);
    if (!monster || !narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    int combat_ongoing = 1, item_used = 0;
    state.current_mode = MODE_COMBAT;
    state.previous_mode = MODE_NARRATIVE;
    
    while (combat_ongoing && player && player->hp > 0 && monster->health > 0) {
        werase(win);
        mvwprintw(win, 2, 2, "Противник: %s", monster->name);
        mvwprintw(win, 3, 2, "HP врага: %d/%d", monster->health, monster->max_health);
        mvwprintw(win, 4, 2, "Ваше HP: %d/%d", player->hp, player->max_hp);
        mvwprintw(win, 5, 2, "Ваше MP: %d/%d", player->mp, player->max_mp);

        mvwprintw(win, 8, 2, "Действия:");
        mvwprintw(win, 9, 2, "A - Физическая атака");
        mvwprintw(win, 10, 2, "M - Магическая атака (тратит 5 MP)");
        mvwprintw(win, 11, 2, "I - Инвентарь");
        mvwprintw(win, 12, 2, "R - Бежать");
        
        wrefresh(win);
        
        int ch = tolower(getch());
        switch (ch) {
            case 'a':
                player_attack(monster, 0);
                if (monster->health > 0) {
                monster_attack_player(monster);
            }
            break;
        
            case 'm':
                player_attack(monster, 1);
            if (monster->health > 0) {
                monster_attack_player(monster);
            }
            break;
        
            case 'i':
                open_inventory_in_combat();
            break;
        
            case 'r':
                mvwprintw(win, 11, 2, "Вы сбежали!");
                current_location = prev_location;
                combat_ongoing = 0;
                wrefresh(win);
                napms(1000);
                break;
        }
        
        if (monster->health <= 0) {
            end_combat(1);
            give_monster_loot(monster);
            combat_ongoing = 0;
            
            item_template *loc = itemdb_find_by_id(&global_db, 400 + location_id);
            if (loc) {
                loc->template.location_template.type = LOC_EMPTY;
            }
        }
        
        if (player && player->hp <= 0) {
            end_combat(0); 
            combat_ongoing = 0;
        }
    }
    
    state.current_mode = MODE_NARRATIVE;

    if (monster) {
        free(monster);
    }
}

static void open_inventory_in_combat(void) {
    if (!player_inv || !narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    int inventory_open = 1;
    int selected_index = 0;
    
    while (inventory_open) {
        werase(win);
        
        mvwprintw(win, 1, 2, "=== Inventory===");
        mvwprintw(win, 2, 2, "Здоровье: %d/%d", player->hp, player->max_hp);
        mvwprintw(win, 3, 2, "Мана: %d/%d", player->mp, player->max_mp);
        mvwhline(win, 4, 2, ACS_HLINE, getmaxx(win) - 4);
        
        inventory_node *current = player_inv->head;
        int index = 0;
        int line = 5;
        
        while (current && line < getmaxy(win) - 5) {
            item_template* template = itemdb_find_by_id(&global_db, current->item_id);
            if (!template) {
                current = current->next;
                index++;
                continue;
            }
            
            if (index == selected_index) {
                wattron(win, A_REVERSE);
            }
            
            char display[80];
            if (current->type == ITEM_CONSUMABLE) {
                snprintf(display, sizeof(display), "  %s x%d", 
                        template->name, current->state.consumable_state.quantity);
            } else {
                snprintf(display, sizeof(display), "  %s", template->name);
            }
            display[sizeof(display) - 1] = '\0';
            
            mvwprintw(win, line, 4, "%s", display);
            
            if (index == selected_index) {
                wattroff(win, A_REVERSE);
            }
            
            current = current->next;
            line++;
            index++;
        }
        
        mvwhline(win, getmaxy(win) - 3, 2, ACS_HLINE, getmaxx(win) - 4);
        mvwprintw(win, getmaxy(win) - 2, 2, "E - Использовать  U - Закрыть инвентарь");
        
        wrefresh(win);
        
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (selected_index > 0) selected_index--;
                break;
                
            case KEY_DOWN:
                if (selected_index < player_inv->count - 1) selected_index++;
                break;
                
            case 'e':
            case 'E':
                inventory_node *node = inventory_get_node_at_index(player_inv, selected_index);
                if (node && node->type == ITEM_CONSUMABLE) {
                    if (inventory_use_consumable(player, player_inv, node, &global_db)) {
                        mvwprintw(win, getmaxy(win) - 1, 2, "Предмет использован!");
                        wrefresh(win);
                        napms(1000);
                        return;
                    }
                } else if (node && node->type == ITEM_ARTIFACT) {
                    mvwprintw(win, getmaxy(win) - 1, 2, "В бою нельзя экипировать артефакты!");
                    wrefresh(win);
                    napms(1500);
                }
                break;
                
            case 'u':
            case 'U':
            case 'i':
            case 'I':
                inventory_open = 0;
                break;
        }
    }
}

void player_attack(Monster* monster, int use_magic) {
    if (!player || !monster || !narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    int dice_roll = roll_dice();
    int damage = calculate_player_damage(player, use_magic, dice_roll);
    
    if (damage > 0) {
        monster->health -= damage;
        mvwprintw(win, 11, 2, "Вы бросаете кубик: %d", dice_roll);
        if (use_magic) {
            mvwprintw(win, 12, 2, "Фаербол! Урон: %d", damage);
        } else {
            mvwprintw(win, 12, 2, "Вы нанесли урона: %d", damage);
        }
    } else if (use_magic && player->mp < 5) {
        mvwprintw(win, 11, 2, "Недостаточно маны");
    } else {
        mvwprintw(win, 11, 2, "Промах");
    }
    
    wrefresh(win);
    napms(1000);
}

void monster_attack_player(Monster* monster) {
    if (!player || !monster || !narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    int dice_roll = roll_dice();
    
    if (check_dodge(player)) {
        mvwprintw(win, 17, 2, "Вы уклонились от атаки.");
    } else {
        int damage = calculate_monster_damage(monster, player, dice_roll);
        player->hp -= damage;
        if (player->hp < 0) player->hp = 0;
        
        mvwprintw(win, 17, 2, "%s атакует!", monster->name);
        mvwprintw(win, 18, 2, "Вы получаете %d урона!", damage);
    }
    
    wrefresh(win);
    napms(1000);
}

void end_combat(int victory) {
    if (!narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    
    if (victory) {
        mvwprintw(win, 17, 2, "ПОБЕДА!");
        
        int exp_gained = 10 + (current_location * 5);
        if (player) {
            player->exp += exp_gained;
            mvwprintw(win, 18, 2, "Получено опыта: %d", exp_gained);
        }
        
        check_level_up();
    } else {
        game_over();
    }
    
    wrefresh(win);
    getch();
}

static void create_inventory_window(void) {
    if (inventory_win) {
        return;
    }
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    inventory_win = make_new_win(1, 1, max_y - 2, max_x - 2, "==Inventory==");
    
    if (player_inv && inventory_win) {
        player_inv->win = inventory_win->overlay;
    }
}

static void destroy_inventory_window(void) {
    if (!inventory_win) return;

    if (inventory_win->panel) {
        del_panel(inventory_win->panel);
    }
    if (inventory_win->overlay) {
        delwin(inventory_win->overlay);
    }
    if (inventory_win->decoration) {
        delwin(inventory_win->decoration);
    }
    if (inventory_win->background) {
        delwin(inventory_win->background);
    }
    
    free(inventory_win);
    inventory_win = NULL;
    
    if (player_inv) {
        player_inv->win = NULL;
    }
    
    update_panels();
    doupdate();
}

void open_inventory(void) {
    if (!player_inv) return;
    
    if (inventory_win) {
        destroy_inventory_window();
        return;
    }
    create_inventory_window();
    if (!inventory_win) return;
    
    state.inventory_selected_index = 0;
    int inventory_open = 1;
    
    while (inventory_open) {
        display_inventory(player_inv, &global_db, player, state.inventory_selected_index);
        int ch = getch();
            
        switch (ch) {
            case KEY_UP:
                if (state.inventory_selected_index > 0)
                    state.inventory_selected_index--;
                break;
                
            case KEY_DOWN:
                if (state.inventory_selected_index < player_inv->count - 1)
                    state.inventory_selected_index++;
                break;
                
            case 'e':
            case 'E':
                use_item_from_inventory(state.inventory_selected_index);
                break;
                    
            case 'd':
            case 'D':
                drop_item_from_inventory(state.inventory_selected_index);
                break;
            case 'i':
            case 'I':
                inventory_open = 0;
                break;
        }
    }
    destroy_inventory_window();
}

void use_item_from_inventory(int index) {
    if (!player_inv) return;
    
    inventory_node *node = inventory_get_node_at_index(player_inv, index);
    if (!node) return;
    
    item_template *item = itemdb_find_by_id(&global_db, node->item_id);
    if (!item) return;
    
    if (node->type == ITEM_ARTIFACT) {
        if (node->state.artifact_state.is_equipped) {
            for (int i = 0; i < MAX_EQUIPPED; i++) {
                if (player_inv->equipped[i] == node) {
                    inventory_unequip_artifact(player_inv, (equipment_slot)i);
                    break;
                }
            }
        } else {
            for (int i = 0; i < MAX_EQUIPPED; i++) {
                if (!player_inv->equipped[i]) {
                    inventory_equip_artifact(player_inv, node, (equipment_slot)i);
                    break;
                }
            }
        }
    } else {
        inventory_use_consumable(player, player_inv, node, &global_db);
    }
    
    calculate_stats_from_equipment(player, player_inv, &global_db);
}

void drop_item_from_inventory(int index) {
    if (!player_inv) return;
    
    inventory_node *node = inventory_get_node_at_index(player_inv, index);
    if (!node) return;
    
    inventory_remove_item(player_inv, node);
    
    if (state.inventory_selected_index >= player_inv->count) {
        state.inventory_selected_index = player_inv->count - 1;
    }
}

void game_over() {
    if (!narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    
    werase(win);
    
    wattron(win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win, 2, max_x/2 - 8, "=== ВЫ УМЕРЛИ ===");
    mvwprintw(win, 4, max_x/2-15, "Ваше приключение завершилось...");
    mvwprintw(win, 6, max_x/2-15, "Хотите сыграть заново?");
    mvwprintw(win, 8, max_x/2-15, "Y - Да, начать заново");
    mvwprintw(win, 9, max_x/2-15, "N - Нет, выйти из игры");
    wattroff(win, COLOR_PAIR(2) | A_BOLD);
    
    wrefresh(win);
    
    int choice = 0;
    while (!choice) {
        int ch = tolower(getch());
        if (ch == 'y') {
            choice = 1;
        } else if (ch == 'n') { 
            choice = 2;
        }
    }
    
    if (choice == 1) {
        state.restart_flag = 1;  // ← ИСПРАВЛЕНО: используем restart_flag
    } else {
        state.quit_flag = 1;
    }
}

void victory_screen(void) {
    if (!narrative_win || !narrative_win->overlay) return;
    
    final(narrative_win->overlay);
    state.quit_flag = 1;
}

void save_game(void) {
    if (!player) return;
    
    save_hero(player, player->name);
    
    WINDOW *win = narrative_win ? narrative_win->overlay : NULL;
    if (win) {
        werase(win);
        mvwprintw(win, 1, 2, "Игра сохранена!");
        mvwprintw(win, 2, 2, "Нажмите любую клавишу...");
        wrefresh(win);
        getch();
    }
}

int get_location_connection(int from, int to) {
    if (from >= 0 && from < 7 && to >= 0 && to < 7) {
        return location_connections[from][to];
    }
    return 0;
}

void print_hint(WINDOW *win, int line, int max_y, item_template *loc){
    mvwprintw(win, line++, 2, "Доступные пути:");
        
    int exit_count = 0;
    for (int i = 0; i < 7; i++) {
        if (location_connections[current_location][i] && line < max_y - 2) {
            mvwprintw(win, line++, 4, "%d. %s", i, location_names[i]);
            exit_count++;
        }
    }
        
    if (exit_count == 0 && line < max_y - 2) {
        mvwprintw(win, line++, 4, "Нет доступных путей");
    }
        
    line += 2;
    if (line < max_y - 2) {
        switch (loc->template.location_template.type) {
            case LOC_MONSTER:
                mvwprintw(win, line++, 2, "Кажется вас заметил монстр.");
                mvwprintw(win, line++, 2, "A - Атаковать");
                break;
            case LOC_TREASURE:
                if (line < max_y - 2) {
                    mvwprintw(win, line++, 2, "T - Взять");
                }
                break;
        }
    }
        
    line += 2;
    if (line < max_y - 2) {
        mvwprintw(win, line++, 2, "I - Инвентарь  Q - Выход");
    }
    wrefresh(win);
}

static void give_monster_loot(Monster* monster) {
    if (!monster || !player_inv || !narrative_win || !narrative_win->overlay) return;
    int loot_found = 0,max_x, max_y;
    WINDOW *win = narrative_win->overlay;
    werase(win);

    getmaxyx(win, max_y, max_x);
    
    mvwprintw(win, max_y/2, max_x/2 - 10, "Вы победили %s!", monster->name);
    
    // Проверяем, если это последний монстр (лабаз)
    if (current_location == 6) { // Лабаз в корнях
        mvwprintw(win, max_y/2 + 2, max_x/2 - 10, "Вы достигли своей цели!");
        mvwprintw(win, max_y/2 + 3, max_x/2 - 15, "Поздравляем с завершением игры!");
        wrefresh(win);
        napms(2000);
        
        // Показываем финальную заставку и спрашиваем о перезапуске
        int want_restart = final(win);
        
        if (want_restart) {
            state.restart_flag = 1;  // ← Устанавливаем флаг перезапуска
        } else {
            state.quit_flag = 1;     // ← Устанавливаем флаг выхода
        }
        return; // Выходим из функции
    }
    
    int line = 4;
    for (int i = 0; i < 3; i++) {
        if (monster->loot_id[i] != 0) {
            item_template *item = itemdb_find_by_id(&global_db, monster->loot_id[i]);
            if (item) {
                if (inventory_add_item_by_id(player_inv, &global_db, monster->loot_id[i], 1)) {
                    mvwprintw(win, line++, 4, "Получено: %s", item->name);
                    loot_found = 1;
                }
            }
        }
    }
    
    if (!loot_found) {
        mvwprintw(win, line++, 4, "Лут не найден");
    }
    
    mvwprintw(win, line + 2, 2, "Нажмите любую клавишу...");
    wrefresh(win);
    getch();
}