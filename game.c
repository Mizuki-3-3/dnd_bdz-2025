#include "interface.h"
#include "history.h"
#include "hero.h"
#include "inventory.h"
#include "database.h"
#include "monster.h"
#include "location.h"
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>

curw *narrative_win = NULL;  
curw *inventory_win = NULL;
curw *equipment_win = NULL;
item_database global_db;
Hero *player = NULL;
inventory *player_inv = NULL;
game_state state;
int current_location = 0;
int prev_location = 0;

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
static void combat_with_monster(int monster_id);
static void take_treasure(int location_id);
static void handle_location_action(int location_id);
static void start_combat(int location_id);
static void player_attack(Monster* monster);
static void monster_attack_player(Monster* monster);
static void end_combat(int victory);
static void open_inventory(void);
static void use_item_from_inventory(int index);
static void drop_item_from_inventory(int index);
static void game_over(void);
static void victory_screen(void);
static void check_level_up(void);
static void move_to_location(int new_location);
void show_location_description(int location_id);

// Новые функции для управления окном инвентаря
static void create_inventory_window(void);
static void destroy_inventory_window(void);

void create_game_windows(void) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Окно повествования - ВЕСЬ ЭКРАН
    narrative_win = make_new_win(1, 1, max_y - 2, max_x - 2, "==Dungeon==");
    
    // Окно инвентаря создается только при открытии инвентаря
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
            mvwprintw(win, 15, 14, "   ");  // Очищаем область ввода
            wmove(win, 15, 14);  // Возвращаем курсор на место ввода
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
    
    // Копируем имя в структуру героя
    strncpy(player->name, name, MAX_NAME_LENGTH - 1);
    player->name[MAX_NAME_LENGTH - 1] = '\0';
    
    // Даём стартовые предметы
    inventory_add_item_by_id(player_inv, &global_db, 201, 1); // Кинжал
    inventory_add_item_by_id(player_inv, &global_db, 202, 1);
    inventory_add_item_by_id(player_inv, &global_db, 203, 1);
    
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
    // позже загрузить инвентарь из отдельного файла
    
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
        // Обновляем заголовок окна с названием локации
        update_window_titles(loc);
        const char *description = loc->template.location_template.description;
        print_wrapped_text(win, description);
        getyx(win, line, col);
        print_hint(win, line+2, max_y, loc); //+2 чтоб не слипались строки
        }
}

void combat_with_monster(int location_id) {
    start_combat(location_id);
}

void take_treasure(int location_id) {
    item_template *loc = itemdb_find_by_id(&global_db, 400 + location_id);
    if (!loc || loc->template.location_template.type != LOC_TREASURE) return;
    
    // Генерация сокровища в зависимости от локации
    int treasure_id = 0;
    switch (location_id) {
        case 2:  // Пещера масок
            treasure_id = 201 + (rand() % 3); // Артефакты 201-203
            break;
        case 5:  // Родник
            treasure_id = 103; // Аптечка
            break;
        default:
            treasure_id = 103; // По умолчанию аптечка
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
    
    // Меняем тип локации на пустую
    loc->template.location_template.type = LOC_EMPTY;
}

void game_loop(void) {
    // Инициализация
    srand(time(NULL));
    itemdb_init(&global_db);
    init_default_items(&global_db);
    
    create_game_windows();
    // Окно ввода имени
    echo();
    char player_name[MAX_NAME_LENGTH];
    memset(player_name, 0, sizeof(player_name));
    
    if (narrative_win && narrative_win->overlay) {
        werase(narrative_win->overlay);
        mvwprintw(narrative_win->overlay, 1, 2, "Введите имя героя:");
        mvwprintw(narrative_win->overlay, 2, 2, "> ");
        wrefresh(narrative_win->overlay);
        wgetnstr(narrative_win->overlay, player_name, MAX_NAME_LENGTH-1);
    }
    noecho();

    int load_result = 0;
    char save_filename[256];
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
            }
        }
    }
    if (!load_result) {
        // Новая игра - выбор класса
        if (narrative_win && narrative_win->overlay) {
            int class = choose_class(narrative_win->overlay);
            init_new_game(player_name, class);
            // Показываем вступление
            chapter1(narrative_win->overlay);
        }
    }

    update_panels();
    doupdate();
    state.current_mode = MODE_NARRATIVE;
    state.previous_mode = MODE_NARRATIVE;
    state.inventory_selected_index = 0;
    state.quit_flag = 0;
    item_template *loc = itemdb_find_by_id(&global_db, 400);
    
    // Показать начальную локацию
    if (loc) {
        update_window_titles(loc);
        show_current_location(loc);
    }
    
    while (!state.quit_flag) {
        switch (state.current_mode) {
            case MODE_NARRATIVE:
                loc = itemdb_find_by_id(&global_db, 400+current_location);
                show_current_location(loc);
                
                // Обработка ввода
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
                        // Открыть/закрыть инвентарь
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
    
    // Сохранение перед выходом
    if (player) {
        save_hero(player, player_name);
        destroy_hero(player);
    }
    if (player_inv) {
        free_inventory(player_inv);
    }
    
    // Уничтожаем окно инвентаря если оно существует
    if (inventory_win) {
        destroy_inventory_window();
    }
}

int roll_dice(int sides) {
    return (rand() % sides) + 1;
}

int calculate_damage(int base, int dice_roll) {
    return base + dice_roll;
}

void check_level_up(void) {
    if (!player || !narrative_win || !narrative_win->overlay) return;
    
    if (player->exp >= player->exp_to_next) {
        // Увеличиваем уровень
        player->level++;
        player->exp -= player->exp_to_next;
        player->exp_to_next = (int)(player->exp_to_next * 1.5);
        
        // Улучшаем характеристики
        player->max_hp += 10;
        player->hp = player->max_hp;
        player->max_mp += 5;
        player->mp = player->max_mp;
        
        // Увеличиваем базовые характеристики
        player->base_strength += 2;
        player->base_dexterity += 1;
        player->base_magic += 1;
        
        // Обновляем текущие характеристики
        player->strength = player->base_strength;
        player->dexterity = player->base_dexterity;
        player->magic = player->base_magic;
        
        // Обновляем емкость инвентаря
        inventory_update_capacity(player_inv, player->level);
        
        // Показываем сообщение
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
        // Нет связи между локациями
        if (narrative_win && narrative_win->overlay) {
            WINDOW *win = narrative_win->overlay;
            werase(win);
            box(win, 0, 0);
            mvwprintw(win, 1, 2, "Невозможно перейти в эту локацию отсюда!");
            mvwprintw(win, 2, 2, "Нажмите любую клавишу...");
            wrefresh(win);
            getch();
        }
        return;
    }
    
    prev_location = current_location;
    current_location = new_location;
    
    if (player) {
        player->current_location = current_location;
        player->prev_location = prev_location;
    }
    
    // Обновляем заголовок окна с названием новой локации
    item_template *loc = itemdb_find_by_id(&global_db, 400 + current_location);
    update_window_titles(loc);
    
    // Обрабатываем события в новой локации
    handle_location_action(current_location);
}

void handle_location_action(int location_id) {
    item_template *loc = itemdb_find_by_id(&global_db, 400 + location_id);
    if (!loc) return;
    
    switch (loc->template.location_template.type) {
        case LOC_MONSTER:
            // Монстр появится при попытке атаковать
            break;
            
        case LOC_TREASURE:
            // Сокровище можно взять клавишей T
            break;
            
        case LOC_EMPTY:
            // Пустая локация - ничего не происходит
            break;
    }
}

void start_combat(int location_id) {
    // Если окно инвентаря открыто - закрываем его
    if (inventory_win) {
        destroy_inventory_window();
    }
    
    // Создаем монстра для этой локации
    Monster *monster = create_monster(location_id);
    if (!monster || !narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    int combat_ongoing = 1;
    state.current_mode = MODE_COMBAT;
    state.previous_mode = MODE_NARRATIVE;
    
    while (combat_ongoing && player && player->hp > 0 && monster->health > 0) {
        werase(win);

        mvwprintw(win, 2, 2, "Противник: %s", monster->name);
        mvwprintw(win, 3, 2, "Здоровье врага: %d/%d", 
                 monster->health, monster->level * 3);
        mvwprintw(win, 4, 2, "Ваше здоровье: %d/%d", 
                 player->hp, player->max_hp);
        
        // Показываем доступные действия
        mvwprintw(win, 6, 2, "Действия:");
        mvwprintw(win, 7, 2, "A - Атаковать");
        mvwprintw(win, 8, 2, "I - Использовать предмет");
        mvwprintw(win, 9, 2, "R - Бежать");
        
        wrefresh(win);
        
        int ch = tolower(getch());
        switch (ch) {
            case 'a':
                player_attack(monster);
                if (monster->health > 0) {
                    monster_attack_player(monster);
                }
                break; 
            case 'r':
                // Отступление
                mvwprintw(win, 11, 2, "Вы отступаете!");
                current_location = prev_location;
                combat_ongoing = 0;
                wrefresh(win);
                napms(1000);
                break;
        }
        
        // Проверяем конец боя
        if (monster->health <= 0) {
            end_combat(1); // Победа
            combat_ongoing = 0;
            
            // Меняем тип локации на пустую
            item_template *loc = itemdb_find_by_id(&global_db, 400 + location_id);
            if (loc) {
                loc->template.location_template.type = LOC_EMPTY;
            }
        }
        
        if (player && player->hp <= 0) {
            end_combat(0); // Поражение
            combat_ongoing = 0;
            game_over();
        }
    }
    
    state.current_mode = MODE_NARRATIVE;
    
    // Освобождаем память монстра
    if (monster) {
        free(monster);
    }
}

void player_attack(Monster* monster) {
    if (!player || !monster || !narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    int dice_roll = roll_dice(6);
    int damage = calculate_damage(player->strength, dice_roll);
    
    monster->health -= damage;
    
    mvwprintw(win, 11, 2, "Вы бросаете кубик: %d", dice_roll);
    mvwprintw(win, 12, 2, "Вы наносите %d урона!", damage);
    wrefresh(win);
    napms(1000);
}

void monster_attack_player(Monster* monster) {
    if (!player || !monster || !narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    int dice_roll = roll_dice(6);
    int damage = calculate_damage(monster->strength, dice_roll);
    
    player->hp -= damage;
    if (player->hp < 0) player->hp = 0;
    
    mvwprintw(win, 14, 2, "Монстр бросает кубик: %d", dice_roll);
    mvwprintw(win, 15, 2, "Монстр наносите %d урона!", damage);
    wrefresh(win);
    napms(1000);
}

void end_combat(int victory) {
    if (!narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    
    if (victory) {
        mvwprintw(win, 17, 2, "ПОБЕДА!");
        
        // Даем опыт
        int exp_gained = 10 + (current_location * 5);
        if (player) {
            player->exp += exp_gained;
            mvwprintw(win, 18, 2, "Получено опыта: %d", exp_gained);
        }
        
        check_level_up();
    } else {
        mvwprintw(win, 17, 2, "ВЫ ПРОИГРАЛИ!");
    }
    
    mvwprintw(win, 20, 2, "Нажмите любую клавишу...");
    wrefresh(win);
    getch();
}

// Создание окна инвентаря
static void create_inventory_window(void) {
    if (inventory_win) {
        return;
    }
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Создаем окно инвентаря в правой половине экрана
    inventory_win = make_new_win(1, 1, max_y - 2, max_x - 2, "==Inventory==");
    
    if (player_inv && inventory_win) {
        player_inv->win = inventory_win->overlay;
    }
}

static void destroy_inventory_window(void) {
    if (!inventory_win) return;
    
    // Удаляем панель
    if (inventory_win->panel) {
        del_panel(inventory_win->panel);
    }
    
    // Удаляем окна
    if (inventory_win->overlay) {
        delwin(inventory_win->overlay);
    }
    if (inventory_win->decoration) {
        delwin(inventory_win->decoration);
    }
    if (inventory_win->background) {
        delwin(inventory_win->background);
    }
    
    // Освобождаем память структуры
    free(inventory_win);
    inventory_win = NULL;
    
    // Устанавливаем player_inv->win в NULL
    if (player_inv) {
        player_inv->win = NULL;
    }
    
    // Обновляем панели
    update_panels();
    doupdate();
}

void open_inventory(void) {
    if (!player_inv) return;
    
    // Если окно инвентаря уже существует - закрываем его
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
        // Экипировка/снятие артефакта
        if (node->state.artifact_state.is_equipped) {
            // Снимаем
            for (int i = 0; i < MAX_EQUIPPED; i++) {
                if (player_inv->equipped[i] == node) {
                    inventory_unequip_artifact(player_inv, (equipment_slot)i);
                    break;
                }
            }
        } else {
            // Надеваем в первый свободный слот
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

void game_over(void) {
    if (!narrative_win || !narrative_win->overlay) return;
    
    WINDOW *win = narrative_win->overlay;
    werase(win);
    box(win, 0, 0);
    
    mvwprintw(win, 1, 2, "=== КОНЕЦ ИГРЫ ===");
    mvwprintw(win, 3, 2, "Ваше приключение завершилось...");
    mvwprintw(win, 5, 2, "Нажмите любую клавишу для выхода");
    
    wrefresh(win);
    getch();
    
    state.quit_flag = 1;
}

void victory_screen(void) {
    if (!narrative_win || !narrative_win->overlay) return;
    
    final(narrative_win->overlay);
    state.quit_flag = 1;
}

void save_game(void) {
    if (!player) return;
    
    // Сохраняем героя
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


void update_window_titles(item_template *loc) {
    if (!narrative_win) return;
    
    // Обновляем заголовок основного окна (название локации)
    WINDOW *decoration = narrative_win->decoration;
    werase(decoration);
    box(decoration, 0, 0);
    
    // Форматируем заголовок
    char title[100];
    if (loc && loc->name) {
        snprintf(title, sizeof(title), "== %s ==", loc->name);
    } else {
        snprintf(title, sizeof(title), "==Dungeon==");
    }
    
    tui_win_label(decoration, title, 0);
    
    // Обновляем панели
    update_panels();
    doupdate();
}
void print_hint(WINDOW *win, int line, int max_y, item_template *loc){
    mvwprintw(win, line++, 2, "Доступные пути:");
        
    int exit_count = 0;
    for (int i = 0; i < 7; i++) {
        if (location_connections[current_location][i] && i != prev_location && line < max_y - 2) {
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
                mvwprintw(win, line++, 2, "A - Атаковать");
                if (line < max_y - 2) {
                    mvwprintw(win, line++, 2, "R - Отступить");
                }
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