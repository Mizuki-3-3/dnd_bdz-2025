#include "game.h"
#include <stdio.h>
#include <time.h>

// Function to initialize the game
Game* init_game() {
    Game *game = malloc(sizeof(Game));
    if (!game) return NULL;

    srand(time(NULL));

    itemdb_init(&game->global_itemdb);
    init_default_items(&game->global_itemdb);

    game->player_inventory = create_inventory();
    if (!game->player_inventory) {
        free(game);
        return NULL;
    }

    setup_locations(game);

    return game;
}

void setup_locations(Game *game) {
    game->location_count = 7;
    game->locations = creat_map(&game->location_count);

    game->locations[0]->exits_id[0] = 1;
    game->locations[0]->exits_id[1] = 2;
    game->locations[0]->exit_count = 2;

    game->locations[1]->exits_id[0] = 0;
    game->locations[1]->exits_id[1] = 3;
    game->locations[1]->exit_count = 2;

    game->locations[2]->exits_id[0] = 0;
    game->locations[2]->exits_id[1] = 3;
    game->locations[2]->exit_count = 2;

    game->locations[3]->exits_id[0] = 1;
    game->locations[3]->exits_id[1] = 2;
    game->locations[3]->exits_id[2] = 4;
    game->locations[3]->exits_id[3] = 5;
    game->locations[3]->exit_count = 4;

    game->locations[4]->exits_id[0] = 3;
    game->locations[4]->exits_id[1] = 5;
    game->locations[4]->exit_count = 2;

    game->locations[5]->exits_id[0] = 3;
    game->locations[5]->exits_id[1] = 4;
    game->locations[5]->exits_id[2] = 6;
    game->locations[5]->exit_count = 3;

    game->locations[6]->exits_id[0] = 5;
    game->locations[6]->exit_count = 1;

    strcpy(game->locations[0]->description, "Вы очнулись на холодной гальке. Шумная вода омывает ваши ноги, а по спине ломит от ушибов. Вокруг — непроглядная стена хвойного леса. Отсюда видны два пути: узкая тропка, уходящая в чащу на восток, и звериная тропа, петляющая вдоль реки на запад. Пора выбираться.");
    strcpy(game->locations[1]->description, "Продравшись сквозь стену папоротников, вы натыкаетесь на почерневший сруб, тесно обнятый соснами. Сквозь провалы в крыше видно небо. Внутри пахнет сыростью и зверем. У дальнего угла — груда тряпья и шкур. На обломке алтаря тускло поблёскивает металлический предмет. Тишина неестественна... и кажется, не пуста.");
    strcpy(game->locations[2]->description, "Обогнув огромный валун, вы находите низкий вход в пещеру, скрытый занавесом из корней. Внутри на стенах висят десятки деревянных масок с пустыми глазницами. Под ними на каменных полках лежат забытые вещи. Воздух сухой и неподвижный. Здесь кто-то хранит память... или снаряжение. У одной из самых старых масок на полке лежит аккуратный свёрток.");
    strcpy(game->locations[3]->description, "Вы выходите на тихую развилку. Здесь стоит кривой, полуистлевший столб. Резные знаки на нём стёрлись до неузнаваемости, но под ним лежит старый, вросший в землю камень. Отсюда тропы ведут обратно к реке, к часовенке, к пещере с масками, а также две новые: одна — в густую, тёмную чащу, другая — под уклон, к шуму воды. Пора свериться с внутренней картой.");
    strcpy(game->locations[4]->description, "Чаща внезапно расступается, открывая зловещую картину: несколько огромных сгнивших стволов лежат по кругу, как спицы колеса. Внутри кольца земля чёрная, влажная, а воздух густой от сладковатого запаха грибов. Между бревнами натянуты липкие, похожие на паутину, нити, усыпанные спорами. В центре, на самом крупном бревне, что-то неестественно блестит. Идти дальше — значит нарушить этот больной покой.");
    strcpy(game->locations[5]->description, "Спустившись под уклон, вы находите источник. Ледяная вода бьёт из-под камня, наполняя идеальную каменную чашу. Здесь тихо и безопасно, шум ручья заглушит ваши шаги. На краю чаши лежат несколько гладких камешков — будто кто-то их недавно перебирал. Это место, чтобы перевести дух, напиться и прислушаться к лесу. Отсюда тропа ведёт обратно к развилке и дальше вглубь, туда, где воздух пахнет смолой и сырой землёй.");
    strcpy(game->locations[6]->description, "Вы продираетесь сквозь завесу свисающих корней огромной поваленной сосны — и попадаете в тесное, но сухое пространство под её основанием. Это оно. Ваш тайник. Здесь стоит примитивный алхимический набор, на полке — склянки, а в углу за камнем спрятан просмоленный мешок с самым ценным. Однако тишина здесь кажется настороженной... Слишком тихо. Как будто кто-то уже ждал.");

    strcpy(game->locations[0]->name, "Берег бурной реки");
    strcpy(game->locations[1]->name, "Лесная часовенка");
    strcpy(game->locations[2]->name, "Пещера молчаливых масок");
    strcpy(game->locations[3]->name, "Старый указатель");
    strcpy(game->locations[4]->name, "Круг поваленных гигантов");
    strcpy(game->locations[5]->name, "Холодный родник");
    strcpy(game->locations[6]->name, "Лабаз в корнях");

    game->current_location = 0;
}

int move_to_location(Game *game, int location_id) {
    if (location_id < 0 || location_id >= game->location_count) {
        return 0; 
    }

    int is_connected = 0;
    for (int i = 0; i < game->locations[game->current_location]->exit_count; i++) {
        if (game->locations[game->current_location]->exits_id[i] == location_id) {
            is_connected = 1;
            break;
        }
    }

    if (!is_connected) {
        return 0; 
    }

    game->current_location = location_id;
    return 1; 
}


int do_the_roll() {
    return rand() % 6 + 1;
}

int handle_empty_location(Game *game) {
    if (!game || !game->locations) return 0;

    location *current_loc = game->locations[game->current_location];

    printf("\n%s\n", current_loc->description);
    printf("\nДоступные направления:\n");

    for (int i = 0; i < current_loc->exit_count; i++) {
        int exit_id = current_loc->exits_id[i];
        printf("%d. %s\n", i+1, game->locations[exit_id]->name);
    }

    printf("Выберите направление (1-%d): ", current_loc->exit_count);

    int choice;
    scanf("%d", &choice);

    if (choice < 1 || choice > current_loc->exit_count) {
        printf("Неверный выбор. Попробуйте снова.\n");
        return handle_empty_location(game);
    }

    int target_location = current_loc->exits_id[choice - 1];
    move_to_location(game, target_location);

    return 1;
}

// Handle monster location
int handle_monster_location(Game *game) {
    if (!game || !game->locations) return 0;

    location *current_loc = game->locations[game->current_location];

    Monster monster;
    monster.level = game->hero->level;  // Match monster level to hero level
    monster.health = monster.level * 3;
    monster.strength = monster.level + 2;
    monster.is_killed = 0;

    for (int i = 0; i < MAX_LOOT_COUNT; i++) {
        monster.loot_id[i] = rand() % 10 + 1;  // Random item IDs
    }

    printf("\n%s\n", current_loc->description);
    printf("Вы встречаете монстра! Он готовится к бою.\n");
    printf("Монстр - Уровень: %d, Здоровье: %d, Сила: %d\n",
           monster.level, monster.health, monster.strength);

    printf("\nДоступные действия:\n");
    printf("1. Сражаться\n");
    printf("2. Сбежать\n");
    printf("Ваш выбор: ");

    int choice;
    scanf("%d", &choice);

    if (choice == 1) {
        int battle_result = battle_system(game, &monster);

        if (battle_result == 1) {  // Victory

            int exp_gained = monster.level * 10;
            game->hero->exp += exp_gained;
            printf("Вы получили %d опыта.\n", exp_gained);

            if (game->hero->exp >= game->hero->exp_to_next) {
                level_up(game->hero, game->player_inventory);
            }

            printf("\nВы можете забрать с собой:\n");
            for (int i = 0; i < MAX_LOOT_COUNT; i++) {
                if (monster.loot_id[i] > 0) {
                    printf("  - Предмет ID: %d\n", monster.loot_id[i]);
                }
            }
            current_loc->type = LOC_EMPTY;

            return handle_empty_location(game);
        } else if (battle_result == 0) {  // Defeat
            printf("\nВы были побеждены монстром...\n");
            return 0;
        } else {
            printf("\nВы сбежали с поля боя.\n");
            return 1;
        }
    } else if (choice == 2) {
        printf("Вы сбежали из локации.\n");
        return 1;
    } else {
        printf("Неверный выбор. Попробуйте снова.\n");
        return handle_monster_location(game);
    }

    return 1;
}

// Handle treasure location
int handle_treasure_location(Game *game) {
    if (!game || !game->locations) return 0;

    location *current_loc = game->locations[game->current_location];

    printf("\n%s\n", current_loc->description);

    // Generate random treasures
    printf("Доступные сокровища:\n");
    for (int i = 0; i < 3; i++) {
        int item_id = rand() % 10 + 1;
        printf("  - Предмет ID: %d\n", item_id);
    }

    printf("\nДоступные действия:\n");
    printf("1. чтобы подобрать нажмите 'T'\n");
    printf("2. Перейти в соседнюю локацию\n");
    printf("Ваш выбор: ");

    int choice;
    scanf("%d", &choice);

    if (choice == 1) {
        for (int i = 0; i < 3; i++) {
            int item_id = rand() % 10 + 1;
            inventory_add_item_by_id(game->player_inventory, &game->global_itemdb, item_id, 1);
        }

        current_loc->type = LOC_EMPTY;
    } else if (choice == 2) {
    
        current_loc->type = LOC_EMPTY;
    } else {
        printf("Неверный выбор. Попробуйте снова.\n");
        return handle_treasure_location(game);
    }

    return handle_empty_location(game);
}


