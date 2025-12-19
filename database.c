#include "database.h"
#include <stdlib.h>
#include <string.h>

void itemdb_init(item_database* db) {
    if (!db) return;
    db->count = 0;

    for (int i = 0; i < MAX_ITEMS; i++) {
        db->items[i].id = 0;
        db->items[i].name[0] = '\0';
        db->items[i].description[0] = '\0';
        memset(&db->items[i].template, 0, sizeof(db->items[i].template));
    }
}

item_template* itemdb_create_artifact(item_database* db, const char* name, const char* desc, artifact_type type, int weight_bonus, int magic_bonus, int strength_bonus, int dexterity_bonus, int id) {
    if (db->count >= MAX_ITEMS) return NULL;
    
    item_template* item = &db->items[db->count];
    memset(item, 0, sizeof(item_template));
    
    item->id = 200 + id; 
    
    if (name) {
        strncpy(item->name, name, MAX_NAME_LENGTH - 1);
        item->name[MAX_NAME_LENGTH - 1] = '\0';
    }
    
    if (desc) {
        strncpy(item->description, desc, MAX_DESC_LENGTH - 1);
        item->description[MAX_DESC_LENGTH - 1] = '\0';
    }
    
    item->template.artifact_template.weight_bonus = weight_bonus;
    item->template.artifact_template.magic_bonus = magic_bonus;
    item->template.artifact_template.strength_bonus = strength_bonus;
    item->template.artifact_template.dexterity_bonus = dexterity_bonus;
    item->template.artifact_template.type = type;

    db->count++;
    return item;
}

item_template* itemdb_create_consumable(item_database* db, const char* name, consumable_type type, int power, int duration, int id) {
    if (db->count >= MAX_ITEMS) return NULL;
    item_template* item = &db->items[db->count];
    memset(item, 0, sizeof(item_template));
    
    item->id = 100 + id;
    
    if (name) {
        strncpy(item->name, name, MAX_NAME_LENGTH - 1);
        item->name[MAX_NAME_LENGTH - 1] = '\0';
    }
    item->template.consumable_template.type = type;
    item->template.consumable_template.power = power;
    item->template.consumable_template.duration = duration;
    
    db->count++;
    return item;
}

item_template* itemdb_create_location(item_database* db, const char* name, const char* desc, int exits_count, loc_type type, int id){
    if (db->count >= MAX_ITEMS) return NULL;
    
    item_template* loc = &db->items[db->count];
    
    memset(loc, 0, sizeof(item_template));
    
    loc->id = 400 + id;
    
    if (name) {
        strncpy(loc->name, name, MAX_NAME_LENGTH - 1);
        loc->name[MAX_NAME_LENGTH - 1] = '\0';
    }
    if (desc) {
        strncpy(loc->template.location_template.description, desc, MAX_DESC_LENGTH - 1);
        loc->template.location_template.description[MAX_DESC_LENGTH - 1] = '\0';
    }
    
    loc->template.location_template.exit_count = exits_count;
    loc->template.location_template.type = loc->template.location_template.original_type = type;
    
    db->count++;
    return loc;
}

item_template* itemdb_find_by_id(item_database* db, int id) {    
    for (int i = 0; i < db->count; i++) {
        if (db->items[i].id == id) {
            return &db->items[i];
        }
    }
    return NULL;
}

void init_default_items(item_database* db) {
    if (!db) return;
    
    // Создаем временные переменные для результатов
    item_template* temp_item = NULL;
    
    // Все предметы игры здесь
    temp_item = itemdb_create_location(db, "Берег бурной реки", "Вы очнулись на холодной гальке. Шумная вода омывает ваши ноги, а спину ломит от ушибов. Вокруг — непроглядная стена хвойного леса. Отсюда видны два пути: узкая тропка, уходящая в чащу на восток, и звериная тропа, петляющая вдоль реки на запад.\n Пора выбираться.",2, LOC_EMPTY, 0);
    if (!temp_item) return;
    temp_item = itemdb_create_location(db, "Лесная часовенка", "Продравшись сквозь стену папоротников, вы натыкаетесь на почерневший сруб, тесно обнятый соснами. Сквозь провалы в крыше видно небо. Внутри пахнет сыростью и зверем. У дальнего угла — груда тряпья и шкур. На обломке алтаря тускло поблёскивает металлический предмет. Тишина неестественна... и кажется, не пуста.",2, LOC_MONSTER, 1);
    if (!temp_item) return;
    temp_item = itemdb_create_location(db, "Пещера молчаливых масок", "Обогнув огромный валун, вы находите низкий вход в пещеру, скрытый занавесом из корней. Внутри на стенах висят десятки деревянных масок с пустыми глазницами. Под ними на каменных полках лежат забытые вещи. Воздух сухой и неподвижный. Здесь кто-то хранит память... или снаряжение. У одной из самых старых масок на полке лежит аккуратный свёрток.",2, LOC_TREASURE, 2);
    if (!temp_item) return;
    temp_item = itemdb_create_location(db, "Старый указатель", "Вы выходите на тихую развилку. Здесь стоит кривой, полуистлевший столб. Резные знаки на нём стёрлись до неузнаваемости, но под ним лежит старый, вросший в землю камень. Отсюда тропы ведут обратно к реке, к часовенке, к пещере с масками, а также две новые: одна — в густую, тёмную чащу, другая — под уклон, к шуму воды. Пора свериться с внутренней картой.",2, LOC_EMPTY, 3);
    if (!temp_item) return;
    temp_item = itemdb_create_location(db, "Круг поваленных гигантов", "Чаща внезапно расступается, открывая зловещую картину: несколько огромных сгнивших стволов лежат по кругу, как спицы колеса. Внутри кольца земля чёрная, влажная, а воздух густой от сладковатого запаха грибов. Между бревнами натянуты липкие, похожие на паутину, нити, усыпанные спорами. В центре, на самом крупном бревне, что-то неестественно блестит. Идти дальше — значит нарушить этот больной покой.",2, LOC_MONSTER, 4);
    if (!temp_item) return;
    temp_item = itemdb_create_location(db, "Холодный родник", "Спустившись под уклон, вы находите источник. Ледяная вода бьёт из-под камня, наполняя идеальную каменную чашу. Здесь тихо и безопасно, шум ручья заглушит ваши шаги. На краю чаши лежат несколько гладких камешков — будто кто-то их недавно перебирал. Это место, чтобы перевести дух, напиться и прислушаться к лесу. Отсюда тропа ведёт обратно к развилке и дальше вглубь, туда, где воздух пахнет смолой и сырой землёй.",2, LOC_TREASURE, 5);
    if (!temp_item) return;
    temp_item = itemdb_create_location(db, "Лабаз в корнях", "Вы продираетесь сквозь завесу свисающих корней огромной поваленной сосны — и попадаете в тесное, но сухое пространство под её основанием. Это оно. Ваш тайник. Здесь стоит примитивный алхимический набор, на полке — склянки, а в углу за камнем спрятан просмоленный мешок с самым ценным. Однако тишина здесь кажется настороженной... Слишком тихо.Как будто кто-то уже ждал.",2, LOC_MONSTER, 6);
    if (!temp_item) return;
    // Создаем артефакты
    temp_item = itemdb_create_artifact(db, "Короткий кинжал", "Ржавый, но острый резак. Всегда с тобой. Рукоять липкая от смолы.", ART_WEAPON, 0, 0, 2, 0, 1);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Мокрые сапоги", "Промокшие насквозь. Подошва стоптана, но держится. Хлюпают на каждом шагу.", ART_BOOTS, 0, 0, 0, 2, 2);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Простая рубаха", "Мокрая, грубая ткань. Холодная и тяжёлая, пахнет тиной.", ART_ARMOR, 0, 2, 0, 0,3);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Простые портки", "Промокшие холщовые штаны. Намокли, стали неудобными и тяжёлыми.", ART_PANTS, 0,0,1,-1,4);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Обмотки следопыта", "Плотные, серые шерстяные обмотки, пропитанные воском и хвойной смолой. Быстро сохнут, не шуршат.", ART_PANTS, 0, 0, 0, 0,5);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Сапоги на мягкой подошве", "Поношенные, но качественные сапоги дозорного. Подошва из нескольких слоёв войлока и кожи — идеальна для бесшумного шага.", ART_BOOTS, 0, 0, 0, 0, 6);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Камзол из вощёной кожи", "Короткий, тёмно-зелёный дублет со вставками из упругой, промасленной кожи на груди и плечах. Карманы для метательных ножей.", ART_ARMOR, 0, 1, 1, 6, 7);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Набедренники из грубой шкуры", "Сшиты из полос толстой шкуры, снятой с оборотня. Волосы снаружи, мех — к телу для тепла. Пахнут дымом и зверем.", ART_PANTS, 0, 0, 5, 1, 8);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Прочные сапоги из кабаньей кожи", "Тяжёлые, с толстой подошвой, подбитой гвоздями. Были на ногах оборотня и деформировались, но ещё целы.", ART_BOOTS, 0, 0, 4, 3,9);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Туника из свалянной шерсти и кожи", "Грубая, колючая туника, сшитая из шкур нескольких животных. На плечах — дополнительные нашивки из просмолённой кожи для защиты.", ART_ARMOR, 0, -1, 6, 0,10);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Поножи из хитиновых пластин", "Сделаны из твёрдых сегментов панциря грибного паука, скреплённых его же прочными нитями. Тёплые, лёгкие, слегка потрескивают при ходьбе.", ART_PANTS, 0, 5, 0, 1, 11);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Башмаки из плесневой кожи", "Сшиты из высушенной и обработанной кожи жертв паука, пропитаны спорами. Подошва покрыта липким, уже мёртвым мицелием, обеспечивающим бесшумное сцепление.",ART_BOOTS, 0, 4, 3, 0, 12);
    if (!temp_item) return;
    temp_item = itemdb_create_artifact(db, "Безрукавка-кокон", "Плотная, серая ткань, сплетённая из паутины и волокон грибницы. На груди нашит высушенный глазок паука, залитый смолой.", ART_ARMOR, 0, 6, -1, 0, 13);
    if (!temp_item) return;

    // делаем расходники
    temp_item = itemdb_create_consumable(db, "Зелье здоровья", HEALTH_POT, 20, 0, 1);
    if (!temp_item) return;
    temp_item = itemdb_create_consumable(db, "Зелье маны", MANA_POT, 15, 4, 2);
    if (!temp_item) return;
    temp_item = itemdb_create_consumable(db, "Зелье силы", STRENGTH_POT, 4, 4, 3);
    if (!temp_item) return;
    temp_item = itemdb_create_consumable(db, "Зелье ловкости", DEXTERITY_POT, 4, 4, 4);
    if (!temp_item) return;
    temp_item = itemdb_create_consumable(db, "Зелье магии", MAGIC_POT, 4, 4, 5);
    if (!temp_item) return;
}