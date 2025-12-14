#include "database.h"
#include <stdlib.h>

void itemdb_init(item_database* db) {
    if (!db) return;
    db->count = 0;

    for (int i = 0; i < MAX_ITEMS; i++) {
        db->items[i].id = 0;
    }
}

item_template* itemdb_create_artifact(item_database* db, const char* name, const char* desc, int weight_bonus, int magic_bonus, int strength_bonus, int dexterity_bonus, int id) {
    if (!db || db->count >= MAX_ITEMS) return NULL;
    
    item_template* item = &db->items[db->count];

    item->id = 200+id; 
    
    strncpy(item->name, name, MAX_NAME_LENGTH - 1);
    strncpy(item->description, desc, MAX_DESC_LENGTH - 1);
    item->template.artifact_template.weight_bonus = weight_bonus;
    item->template.artifact_template.magic_bonus = magic_bonus;
    item->template.artifact_template.strength_bonus = strength_bonus;
    item->template.artifact_template.dexterity_bonus = dexterity_bonus;
    
    db->count++;
    return item;
}

item_template* itemdb_create_consumable(item_database* db, const char* name, const char* desc, consumable_type type, int power, int duration, int id) {
    if (!db || db->count >= MAX_ITEMS) return NULL;
    
    item_template* item = &db->items[db->count];
    
    item->id = 100+id;
    
    strncpy(item->name, name, MAX_NAME_LENGTH - 1);
    strncpy(item->description, desc, MAX_DESC_LENGTH - 1);
    item->template.consumable_template.type = type;
    item->template.consumable_template.power = power;
    item->template.consumable_template.duration = duration;
    
    db->count++;
    return item;
}


item_template* itemdb_create_monster(item_database* db, const char* name, int level, int id, int loot1, int loot2, int loot3){
    if (!db || db->count >= MAX_ITEMS) return NULL;

    item_template* m = &db->items[db->count];
    
    
    m->id = 300+id;
    strncpy(m->name, name, MAX_NAME_LENGTH - 1);
    m->template.monster_template.is_killed = 0;
    m->template.monster_template.health = level*3;
    m->template.monster_template.level = level;
    m->template.monster_template.strength = level+3;
    m->template.monster_template.loot_id[0] = loot1;
    m->template.monster_template.loot_id[1] = loot2;
    m->template.monster_template.loot_id[2] = loot3;
    return m;
}

item_template* itemdb_create_location(item_database* db, const char* name, const char* desc, int exits_count, loc_type type, int id){
    if (!db || db->count >= MAX_ITEMS) return NULL;
    item_template* loc = &db->items[db->count];
    loc->id = 400+id;
    strncpy(loc->name, name, MAX_NAME_LENGTH - 1);
    strncpy(loc->template.location_template.description, desc, MAX_DESC_LENGTH - 1);
    loc->template.location_template.exit_count = exits_count;
    loc->template.location_template.type = loc->template.location_template.original_type = type;
    return loc;
}

item_template* itemdb_find_by_id(item_database* db, int id) {
    if (!db) return NULL;
    
    for (int i = 0; i <= db->count; i++) {
        if (db->items[i].id == id) {
            return &db->items[i];
        }
    }
    return NULL;
}



void init_default_items(item_database* db) {
    if (!db) return;
    
    // Все предметы игры здесь
    itemdb_create_location(db, "Берег бурной реки", "Вы очнулись на холодной гальке. Шумная вода омывает ваши ноги, а по спине ломит от ушибов. Вокруг — непроглядная стена хвойного леса. Отсюда видны два пути: узкая тропка, уходящая в чащу на восток, и звериная тропа, петляющая вдоль реки на запад. Пора выбираться.",2, LOC_EMPTY, 0);
    itemdb_create_location(db, "Лесная часовенка", "Продравшись сквозь стену папоротников, вы натыкаетесь на почерневший сруб, тесно обнятый соснами. Сквозь провалы в крыше видно небо. Внутри пахнет сыростью и зверем. У дальнего угла — груда тряпья и шкур. На обломке алтаря тускло поблёскивает металлический предмет. Тишина неестественна... и кажется, не пуста.",2, LOC_MONSTER, 1);
    itemdb_create_location(db, "Пещера молчаливых масок", "Обогнув огромный валун, вы находите низкий вход в пещеру, скрытый занавесом из корней. Внутри на стенах висят десятки деревянных масок с пустыми глазницами. Под ними на каменных полках лежат забытые вещи. Воздух сухой и неподвижный. Здесь кто-то хранит память... или снаряжение. У одной из самых старых масок на полке лежит аккуратный свёрток.",2, LOC_TREASURE, 2);
    itemdb_create_location(db, "Старый указатель", "Вы выходите на тихую развилку. Здесь стоит кривой, полуистлевший столб. Резные знаки на нём стёрлись до неузнаваемости, но под ним лежит старый, вросший в землю камень. Отсюда тропы ведут обратно к реке, к часовенке, к пещере с масками, а также две новые: одна — в густую, тёмную чащу, другая — под уклон, к шуму воды. Пора свериться с внутренней картой.",2, LOC_EMPTY, 3);
    itemdb_create_location(db, "Круг поваленных гигантов", "Чаща внезапно расступается, открывая зловещую картину: несколько огромных сгнивших стволов лежат по кругу, как спицы колеса. Внутри кольца земля чёрная, влажная, а воздух густой от сладковатого запаха грибов. Между бревнами натянуты липкие, похожие на паутину, нити, усыпанные спорами. В центре, на самом крупном бревне, что-то неестественно блестит. Идти дальше — значит нарушить этот больной покой.",2, LOC_MONSTER, 4);
    itemdb_create_location(db, "Холодный родник", "Спустившись под уклон, вы находите источник. Ледяная вода бьёт из-под камня, наполняя идеальную каменную чашу. Здесь тихо и безопасно, шум ручья заглушит ваши шаги. На краю чаши лежат несколько гладких камешков — будто кто-то их недавно перебирал. Это место, чтобы перевести дух, напиться и прислушаться к лесу. Отсюда тропа ведёт обратно к развилке и дальше вглубь, туда, где воздух пахнет смолой и сырой землёй.",2, LOC_TREASURE, 5);
    itemdb_create_location(db, "Лабаз в корнях", "Вы продираетесь сквозь завесу свисающих корней огромной поваленной сосны — и попадаете в тесное, но сухое пространство под её основанием. Это оно. Ваш тайник. Здесь стоит примитивный алхимический набор, на полке — склянки, а в углу за камнем спрятан просмоленный мешок с самым ценным. Однако тишина здесь кажется настороженной... Слишком тихо.Как будто кто-то уже ждал.",2, LOC_MONSTER, 6);

    itemdb_create_artifact(db, "Короткий кинжал", "Ржавый, но острый резак. Всегда с тобой. Рукоять липкая от смолы.", 0, 0, 5, 0, 1);
    itemdb_create_artifact(db, "Мокрые сапоги", "Промокшие насквозь. Подошва стоптана, но держится. Хлюпают на каждом шагу.", 5, 0, 0, 0, 2);
    itemdb_create_artifact(db, "Простая рубаха", "Мокрая, грубая ткань. Холодная и тяжёлая, пахнет тиной.", 0, 0, 0, 0,3);
    itemdb_create_artifact(db, "Простые портки", "Промокшие холщовые штаны. Намокли, стали неудобными и тяжёлыми.", 0,0,0,0,4);
    itemdb_create_artifact(db, "Обмотки следопыта", "Плотные, серые шерстяные обмотки, пропитанные воском и хвойной смолой. Быстро сохнут, не шуршат.", 0, 0, 0, 0,5);
    itemdb_create_artifact(db, "Сапоги на мягкой подошве", "Поношенные, но качественные сапоги дозорного. Подошва из нескольких слоёв войлока и кожи — идеальна для бесшумного шага.", 0, 0, 0, 0, 6);
    itemdb_create_artifact(db, "Камзол из вощёной кожи", "Короткий, тёмно-зелёный дублет со вставками из упругой, промасленной кожи на груди и плечах. Карманы для метательных ножей.", 0, 0, 0, 0, 7);
    itemdb_create_artifact(db, "Набедренники из грубой шкуры", "Сшиты из полос толстой шкуры, снятой с оборотня. Волосы снаружи, мех — к телу для тепла. Пахнут дымом и зверем.", 0, 0, 0, 0, 8);
    itemdb_create_artifact(db, "Прочные сапоги из кабаньей кожи", "Тяжёлые, с толстой подошвой, подбитой гвоздями. Были на ногах оборотня и деформировались, но ещё целы.", 0, 0, 0, 0,9);
    itemdb_create_artifact(db, "Туника из свалянной шерсти и кожи", "Грубая, колючая туника, сшитая из шкур нескольких животных. На плечах — дополнительные нашивки из просмолённой кожи для защиты.", 0, 0, 0, 0,10);
    itemdb_create_artifact(db, "Поножи из хитиновых пластин", "Сделаны из твёрдых сегментов панциря грибного паука, скреплённых его же прочными нитями. Тёплые, лёгкие, слегка потрескивают при ходьбе.", 0, 0, 0, 0, 11);
    itemdb_create_artifact(db, "Башмаки из плесневой кожи", "Сшиты из высушенной и обработанной кожи жертв паука, пропитаны спорами. Подошва покрыта липким, уже мёртвым мицелием, обеспечивающим бесшумное сцепление.", 0, 0, 0, 0, 12);
    itemdb_create_artifact(db, "Безрукавка-кокон", "Плотная, серая ткань, сплетённая из паутины и волокон грибницы. На груди нашит высушенный глазок паука, залитый смолой.", 0, 0, 0, 0, 13);


    itemdb_create_consumable(db, "Аптечка", "", HEALTH_POT, 30, 0, 3);
}