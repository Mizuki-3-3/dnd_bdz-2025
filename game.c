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
#include <stdio.h>

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
#define TIME 1000
#define FILE_SIZE 256

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
static void check_level_up(void);
static void move_to_location(int new_location);
static void create_inventory_window(void);
static void destroy_inventory_window(void);
static void open_inventory_in_combat(void);
static void give_monster_loot(Monster* monster);

// Упрощенные функции сохранения/загрузки
static int save_current_game(void);
static int load_saved_game(const char *save_name);

void create_game_windows(void) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Создаем временный указатель для narrative_win
    curw *temp_win = make_new_win(1, 1, max_y - 2, max_x - 2, "==Dungeon==");
    if (!temp_win) {
        // Обработка ошибки: не удалось создать окно
        // Можно вывести сообщение об ошибке
        mvwprintw(stdscr, max_y/2, max_x/2 - 10, "Ошибка создания окна!");
        wrefresh(stdscr);
        napms(TIME);
        return;
    }
    narrative_win = temp_win;
    
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
    // освобождаем старые данные
    if (player) {
        destroy_hero(player);
        player = NULL;
    }
    if (player_inv) {
        free_inventory(player_inv);
        player_inv = NULL;
    }
    
    Hero *temp_hero = create_hero(name, class);
    if (!temp_hero) {
        if (narrative_win && narrative_win->overlay) {
            WINDOW *win = narrative_win->overlay;
            werase(win);
            mvwprintw(win, 1, 2, "ошибка выделения памяти под героя!");
            wrefresh(win);
            napms(TIME);
        }
        return;
    }
    
    inventory *temp_inv = create_inventory();
    if (!temp_inv) {
        destroy_hero(temp_hero);
        if (narrative_win && narrative_win->overlay) {
            WINDOW *win = narrative_win->overlay;
            werase(win);
            mvwprintw(win, 1, 2, "Ошибка создания инвентаря!");
            wrefresh(win);
            napms(TIME);
        }
        return;
    }
    
    // все успешно создано присваиваем переменным значения
    player = temp_hero;
    player_inv = temp_inv;
    
    strncpy(player->name, name, MAX_NAME_LENGTH - 1);
    player->name[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(player_name, name, MAX_NAME_LENGTH - 1);
    
    // штаники сапожки курточки
    if (!inventory_add_item_by_id(player_inv, &global_db, 201, 1) ||!inventory_add_item_by_id(player_inv, &global_db, 202, 1) ||!inventory_add_item_by_id(player_inv, &global_db, 203, 1) ||!inventory_add_item_by_id(player_inv, &global_db, 204, 1)) {
        // ошибка выделения памяти под предметы
        if (narrative_win && narrative_win->overlay) {
            WINDOW *win = narrative_win->overlay;
            mvwprintw(win, getmaxy(win) - 2, 2, "ошибка на предметах!");
            wrefresh(win);
        }
    }

    current_location = 0;
    prev_location = 0;
    
    player->current_location = current_location;
    player->prev_location = prev_location;
}

static int save_current_game(void) {
    if (!player) return 0;
    
    char filename[FILE_SIZE];
    snprintf(filename, sizeof(filename), "%s.save", player_name);
    
    FILE *file = fopen(filename, "wb");
    if (!file) return 0;
    
    if (fwrite(player, sizeof(Hero), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    if (fwrite(&current_location, sizeof(int), 1, file) != 1 ||
        fwrite(&prev_location, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    for (int i = 0; i < 7; i++) {
        item_template *loc = itemdb_find_by_id(&global_db, 400 + i);
        if (loc) {
            int loc_type = loc->template.location_template.type;
            if (fwrite(&loc_type, sizeof(int), 1, file) != 1) {
                fclose(file);
                return 0;
            }
        } else {
            int default_type = LOC_EMPTY; // Если локация не найдена, сохраняем как пустую
            if (fwrite(&default_type, sizeof(int), 1, file) != 1) {
                fclose(file);
                return 0;
            }
        }
    }
    
    int count = player_inv->count;
    if (fwrite(&count, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    inventory_node *current = player_inv->head;
    while (current) {
        if (fwrite(&current->type, sizeof(item_type), 1, file) != 1 ||
            fwrite(&current->item_id, sizeof(int), 1, file) != 1) {
            fclose(file);
            return 0;
        }
        
        if (current->type == ITEM_ARTIFACT) {
            if (fwrite(&current->state.artifact_state.is_equipped, sizeof(int), 1, file) != 1) {
                fclose(file);
                return 0;
            }
        } else if (current->type == ITEM_CONSUMABLE) {
            if (fwrite(&current->state.consumable_state.quantity, sizeof(int), 1, file) != 1) {
                fclose(file);
                return 0;
            }
        }
        
        current = current->next;
    }
    
    fclose(file);
    return 1;
}

static int load_saved_game(const char *save_name) {
    char filename[FILE_SIZE];
    snprintf(filename, sizeof(filename), "%s.save", save_name);
    
    FILE *file = fopen(filename, "rb");
    if (!file) return 0;
    
    Hero *temp_hero = malloc(sizeof(Hero));
    if (!temp_hero) {
        fclose(file);
        return 0;
    }
    
    if (fread(temp_hero, sizeof(Hero), 1, file) != 1) {
        free(temp_hero);
        fclose(file);
        return 0;
    }
    
    int new_current_loc, new_prev_loc;
    if (fread(&new_current_loc, sizeof(int), 1, file) != 1 ||fread(&new_prev_loc, sizeof(int), 1, file) != 1) {
        free(temp_hero);
        fclose(file);
        return 0;
    }
    
    int loc_states[7];
    for (int i = 0; i < 7; i++) {
        if (fread(&loc_states[i], sizeof(int), 1, file) != 1) {
            free(temp_hero);
            fclose(file);
            return 0;
        }
    }
    
    inventory *temp_inv = create_inventory();
    if (!temp_inv) {
        free(temp_hero);
        fclose(file);
        return 0;
    }
    
    int item_count;
    if (fread(&item_count, sizeof(int), 1, file) != 1) {
        free_inventory(temp_inv);
        free(temp_hero);
        fclose(file);
        return 0;
    }
    
    for (int i = 0; i < item_count; i++) {
        item_type type;
        int item_id;
        
        if (fread(&type, sizeof(item_type), 1, file) != 1 || 
            fread(&item_id, sizeof(int), 1, file) != 1) {
            free_inventory(temp_inv);
            free(temp_hero);
            fclose(file);
            return 0;
        }
        
        inventory_node *new_node = malloc(sizeof(inventory_node));
        if (!new_node) {
            free_inventory(temp_inv);
            free(temp_hero);
            fclose(file);
            return 0;
        }
        
        new_node->type = type;
        new_node->item_id = item_id;
        new_node->next = NULL;
        
        if (type == ITEM_ARTIFACT) {
            int is_equipped;
            if (fread(&is_equipped, sizeof(int), 1, file) != 1) {
                free(new_node);
                free_inventory(temp_inv);
                free(temp_hero);
                fclose(file);
                return 0;
            }
            new_node->state.artifact_state.is_equipped = is_equipped;
            
            if (is_equipped) {
                for (int slot = 0; slot < MAX_EQUIPPED; slot++) {
                    if (!temp_inv->equipped[slot]) {
                        temp_inv->equipped[slot] = new_node;
                        break;
                    }
                }
            }
        } else if (type == ITEM_CONSUMABLE) {
            int quantity;
            if (fread(&quantity, sizeof(int), 1, file) != 1) {
                free(new_node);
                free_inventory(temp_inv);
                free(temp_hero);
                fclose(file);
                return 0;
            }
            new_node->state.consumable_state.quantity = quantity;
        }
        
        if (!temp_inv->head) {
            temp_inv->head = new_node;
            temp_inv->tail = new_node;
        } else {
            temp_inv->tail->next = new_node;
            temp_inv->tail = new_node;
        }
        temp_inv->count++;
    }
    
    fclose(file); 
    if (player) {
        destroy_hero(player);
    }
    if (player_inv) {
        free_inventory(player_inv);
    }
    
    player = temp_hero;
    player_inv = temp_inv;
    current_location = new_current_loc;
    prev_location = new_prev_loc;
    strncpy(player_name, save_name, MAX_NAME_LENGTH);
    
    for (int i = 0; i < 7; i++) {
        item_template *loc = itemdb_find_by_id(&global_db, 400 + i);
        if (loc) {
            loc->template.location_template.type = loc_states[i];
        }
    }
    
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
    for (int i = 0; i<3; i++){
        switch (location_id) {
            case 2:
                treasure_id = 205+i;
                break;
            case 5:
                treasure_id = 101+rand()%5;
                if(!i){
                    treasure_id = 101;
                }
                break;
            default:
                treasure_id = 103;
        }
    
        if (inventory_add_item_by_id(player_inv, &global_db, treasure_id, 3)) {
            item_template *treasure = itemdb_find_by_id(&global_db, treasure_id);
            if (treasure && narrative_win && narrative_win->overlay) {
                WINDOW *win = narrative_win->overlay;
                werase(win);
                mvwprintw(win, 1, 2, "Вы нашли: %s", treasure->name);
                mvwprintw(win, 2, 2, "%s", treasure->description);
                mvwprintw(win, 4, 2, "Нажмите любую клавишу");
                wrefresh(win);
                getch();
            }
        }
    }
    loc->template.location_template.type = LOC_EMPTY;
}

void game_loop(void) {
    srand(time(NULL));
    itemdb_init(&global_db);
    init_default_items(&global_db);
    
    create_game_windows();
    
    while (!state.quit_flag) {
        if (narrative_win && narrative_win->overlay) {
            WINDOW *win = narrative_win->overlay;
            werase(win);
            
            int max_y, max_x;
            getmaxyx(win, max_y, max_x);
            
            mvwprintw(win, max_y/2 - 2, max_x/2 - 10, "=== ИГРА ===");
            mvwprintw(win, max_y/2, max_x/2 - 15, "1. Новая игра");
            mvwprintw(win, max_y/2 + 1, max_x/2 - 15, "2. Загрузить игру");
            mvwprintw(win, max_y/2 + 2, max_x/2 - 15, "3. Выход");
            mvwprintw(win, max_y/2 + 4, max_x/2 - 15, "Выберите (1-3): ");
            
            wrefresh(win);
            
            int choice = 0;
            while (!choice) {
                int ch = getch();
                if (ch >= '1' && ch <= '3') {
                    choice = ch - '0';
                }
            }
            
            if (choice == 1) { 
                echo();
                werase(win);
                mvwprintw(win, 1, 2, "Введите имя героя:");
                mvwprintw(win, 2, 2, "> ");
                wrefresh(win);
                wgetnstr(win, player_name, MAX_NAME_LENGTH-1);
                noecho();
                
                chosen_class = choose_class(win);
                init_new_game(player_name, chosen_class);
                chapter1(win);
                break;
            }
            else if (choice == 2) { 
                echo();
                werase(win);
                mvwprintw(win, 1, 2, "Введите имя сохранения:");
                mvwprintw(win, 2, 2, "> ");
                wrefresh(win);
                char save_name[FILE_SIZE];
                wgetnstr(win, save_name, 255);
                noecho();
                
                if (load_saved_game(save_name)) {
                    break; 
                } else {
                    mvwprintw(win, 4, 2, "Ошибка загрузки! Нажмите любую клавишу...");
                    wrefresh(win);
                    getch();
                }
            }
            else if (choice == 3) { 
                state.quit_flag = 1;
                break;
            }
        }
    }
    
    
    while (!state.quit_flag && !state.restart_flag) {
        update_panels();
        doupdate();        
        item_template *loc = itemdb_find_by_id(&global_db, 400 + current_location);
        
        
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
                            
                        case 's': // Сохранить игру
                            if (save_current_game()) {
                                if (narrative_win && narrative_win->overlay) {
                                    WINDOW *win = narrative_win->overlay;
                                    werase(win);
                                    mvwprintw(win, 1, 2, "Игра сохранена");
                                    mvwprintw(win, 2, 2, "Нажмите любую клавишу");
                                    wrefresh(win);
                                    getch();
                                }
                            } else {
                                if (narrative_win && narrative_win->overlay) {
                                    WINDOW *win = narrative_win->overlay;
                                    werase(win);
                                    mvwprintw(win, 1, 2, "Ошибка сохранения");
                                    mvwprintw(win, 2, 2, "Нажмите любую клавишу");
                                    wrefresh(win);
                                    getch();
                                }
                            }
                            break;
                    }
                    break;
                    
                case MODE_COMBAT:
                    break;
            }
            
            update_panels();
            doupdate();
        }
        
        if (player) {
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
        
        if (state.restart_flag) {
            for (int i = 1; i < MAX_LOCATIONS; i++) {
                item_template *loc_item = itemdb_find_by_id(&global_db, 400 + i);
                if (loc_item) {
                    loc_item->template.location_template.type = 
                        loc_item->template.location_template.original_type;
                }
            }
            
            state.restart_flag = 0;
            
            if (narrative_win && narrative_win->overlay) {
                WINDOW* win = narrative_win->overlay;
                werase(win);
                
                int max_y, max_x;
                getmaxyx(win, max_y, max_x);
                
                mvwprintw(win, max_y/2 - 2, max_x/2 - 15, "Игра завершена!");
                mvwprintw(win, max_y/2, max_x/2 - 20, "1. Вернуться в главное меню");
                mvwprintw(win, max_y/2 + 1, max_x/2 - 20, "2. Выйти из игры");
                mvwprintw(win, max_y/2 + 3, max_x/2 - 15, "Выберите пункт (1-2):");
                
                wrefresh(win);
                
                int choice = 0;
                while (!choice) {
                    int ch = getch();
                    if (ch == '1') choice = 1;
                    else if (ch == '2') choice = 2;
                }
                
                if (choice == 1) {
                    state.quit_flag = 0;
                    game_loop();
                    return;
                } else {
                    state.quit_flag = 1;
                }
            }
        }
    }
    
    if (player && strlen(player_name) > 0) {
        save_current_game();
    }
    
    if (player) {
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
        mvwprintw(win, 5, 2, "Нажмите любую клавишу");
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
    Monster *temp_monster = create_monster(location_id);
    if (!temp_monster || !narrative_win || !narrative_win->overlay) {
        if (temp_monster) free(temp_monster);
        return;
    }
    
    Monster *monster = temp_monster;
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
                current_location = prev_location;
                combat_ongoing = 0;
                wrefresh(win);
                break;
        }
        
        if (monster->health <= 0) {
            item_template *loc = itemdb_find_by_id(&global_db, 400 + location_id);
            if (loc) {
                loc->template.location_template.type = LOC_EMPTY;
            }
            end_combat(1);
            give_monster_loot(monster);
            combat_ongoing = 0;
            
            
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
        mvwprintw(win, getmaxy(win) - 2, 2, "U - Использовать  I - Закрыть инвентарь");
        
        wrefresh(win);
        
        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (selected_index > 0) selected_index--;
                break;
                
            case KEY_DOWN:
                if (selected_index < player_inv->count - 1) selected_index++;
                break;
                
            case 'u':
            case 'U':
                inventory_node *node = inventory_get_node_at_index(player_inv, selected_index);
                if (node && node->type == ITEM_CONSUMABLE) {
                    if (inventory_use_consumable(player, player_inv, node, &global_db)) {
                        mvwprintw(win, getmaxy(win) - 1, 2, "Предмет использован!");
                        wrefresh(win);
                        napms(TIME);
                        return;
                    }
                } else if (node && node->type == ITEM_ARTIFACT) {
                    mvwprintw(win, getmaxy(win) - 1, 2, "В бою нельзя экипировать артефакты!");
                    wrefresh(win);
                    napms(TIME);
                }
                break;
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
        mvwprintw(win, 14, 2, "Вы бросаете кубик: %d", dice_roll);
        if (use_magic) {
            mvwprintw(win, 14, 2, "Фаербол! Урон: %d", damage);
        } else {
            mvwprintw(win, 14, 2, "Вы нанесли урона: %d", damage);
        }
    } else if (use_magic && player->mp < 5) {
        mvwprintw(win, 14, 2, "Недостаточно маны");
    } else {
        mvwprintw(win, 14, 2, "Промах");
    }
    
    wrefresh(win);
    napms(TIME);
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
    napms(TIME);
}

void end_combat(int victory) {
    if (!narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    
    if (victory) {
        mvwprintw(win, 17, 2, "ПОБЕДА!");
        
        int exp_gained = 20 + (current_location * 5);
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
    
    curw *temp_win = make_new_win(1, 1, max_y - 2, max_x - 2, "==Inventory==");
    if (!temp_win) {
        return;
    }
    
    inventory_win = temp_win;
    
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
        state.restart_flag = 1;
    } else {
        state.quit_flag = 1;
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
        
    line += 2;
    if (line < max_y - 2) {
        switch (loc->template.location_template.type) {
            case LOC_MONSTER:
                mvwprintw(win, line++, 2, "Кажется вас заметил противник.");
                mvwprintw(win, line++, 2, "A - Атаковать");
                break;
            case LOC_TREASURE:
                if (line < max_y - 2) {
                    mvwprintw(win, line++, 2, "Тут что-то очень привлекательно лежит...");
                    mvwprintw(win, line++, 2, "T - Взять");
                }
                break;
        }
    }
        
    line += 2;
    if (line < max_y - 2) {
        mvwprintw(win, line++, 2, "I - Инвентарь  Q - Выход  S - Сохранить");
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
    
    if (current_location == 6) { // победили дозорного и валим
        mvwprintw(win, max_y/2, max_x/2 - 10, "Вы достигли своей цели!");
        mvwprintw(win, max_y/2, max_x/2 - 15, "Поздравляем с завершением игры!");
        wrefresh(win);
        napms(2*TIME);
        save_current_game();
        int want_restart = final(win);
        
        if (want_restart) {
            state.restart_flag = 1;
        } else {
            state.quit_flag = 1;
        }
        return;
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