#include <stdio.h>
#include <stdlib.h>
#include "inventory.h"
#include "equipment.h"

int main(void) {
    // Инициализация базы данных предметов
    item_database db;
    itemdb_init(&db);
    init_default_items(&db);
    
    // Создаем инвентарь
    inventory *inv = create_inventory();
    if (!inv) {
        printf("Ошибка создания инвентаря!\n");
        return 1;
    }
    
    // Добавляем предметы в инвентарь
    printf("Добавляем предметы...\n");
    
    // Меч (ID: 1) - артефакт
    if (inventory_add_item_by_id(inv, &db, 1, 1)) {
        printf("Добавлен меч\n");
    }
    
    // Броня (ID: 2) - артефакт
    if (inventory_add_item_by_id(inv, &db, 2, 1)) {
        printf("Добавлена броня\n");
    }
    
    // Аптечка (ID: 3??) - расходник
    if (inventory_add_item_by_id(inv, &db, 3, 3)) {
        printf("Добавлена аптечка x3\n");
    }
    
    // Проверяем количество предметов
    printf("Предметов в инвентаре: %d/%d\n", inv->count, inv->max_slots);
    
    // Получаем первый предмет
    inventory_node *first_item = inventory_get_node_at_index(inv, 1);
    if (first_item) {
        item_template *template = itemdb_find_by_id(&db, first_item->item_id);
        if (template) {
            printf("Первый предмет: %s\n", template->name);
        }
    }
    
    // Экипируем меч (первый предмет в слоте оружия)
    if (first_item) {
        if (inventory_equip_artifact(inv, first_item, SLOT_WEAPON)) {
            printf("Меч экипирован!\n");
        }
    }
    
    // Проверяем экипировку
    if (inv->equipped[SLOT_WEAPON]) {
        printf("В слоте оружия: %c");
        item_template *eq_template = itemdb_find_by_id(&db, inv->equipped[SLOT_WEAPON]->item_id);
        if (eq_template) {
            printf("%s\n", eq_template->name);
        }
    }
    
    // Снимаем экипировку
    if (inventory_unequip_artifact(inv, SLOT_WEAPON)) {
        printf("Предмет снят\n");
    }
    
    // Удаляем первый предмет
    if (first_item) {
        if (inventory_remove_item(inv, first_item)) {
            printf("Первый предмет удален\n");
        }
    }
    
    // Обновляем емкость для уровня 5
    inventory_update_capacity(inv, 5);
    printf("Новая емкость инвентаря: %d\n", inv->max_slots);
    
    // Проверяем переполнение
    printf("\nТестируем переполнение...\n");
    int added = 0;
    for (int i = 0; i < 15; i++) {
        if (inventory_add_item_by_id(inv, &db, 3, 1)) {
            added++;
        }
    }
    printf("Добавлено аптечек: %d\n", added);
    printf("Итоговое количество предметов: %d/%d\n", inv->count, inv->max_slots);
    
    // Очищаем память
    free_inventory(inv);
    
    printf("\nТестирование завершено успешно!\n");
    return 0;
}