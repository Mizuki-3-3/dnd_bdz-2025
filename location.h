#ifndef LOCATION_H
#define LOCATION_H

#define MAX_NAME_LENGTH  32
#define MAX_DESC_LENGTH 300
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
void init_map_connections(location** locs, int count);
int can_move_to(location** locs, int from, int to);
void get_available_exits(location** locs, int current, int exits[], int *count);
void change_location_type(location** locs, int loc_id, loc_type new_type);
extern const char* location_names[MAX_LOCATIONS];
extern int location_connections[MAX_LOCATIONS][MAX_LOCATIONS];


#endif