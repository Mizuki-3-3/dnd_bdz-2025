#ifndef LOCATION_H
#define LOCATION_H

#define MAX_EXITS_COUNT 8
typedef enum {
    LOC_EMPTY,
    LOC_MONSTER,
    LOC_TREASURE
}loc_type;

typedef struct location{
    loc_type type;
    loc_type original_type;
    struct Location* exits[MAX_EXITS_COUNT];
    int exit_count;
}location;

location** creat_map(int* loc_count);
void destroy_map(location** locs, int count);


#endif