#ifndef LOCATION_H
#define LOCATION_H

#define MAX_NAME_LENGTH  64
#define MAX_DESC_LENGTH 750
#define MAX_EXITS_COUNT 8
#define MAX_LOCATIONS 7

typedef enum {
    LOC_EMPTY,
    LOC_MONSTER,
    LOC_TREASURE
}loc_type;

typedef struct location{
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESC_LENGTH];
    int id;
    loc_type type;
    loc_type original_type;
    int exits_id[MAX_EXITS_COUNT];
    int exit_count;
}location;

location** creat_map(int* loc_count);
void destroy_map(location** locs, int count);

#endif