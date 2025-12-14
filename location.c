#include "location.h"
#include <stdlib.h>
#include <string.h>

// Create a map of locations
location** creat_map(int* loc_count) {
    if (!loc_count || *loc_count <= 0) return NULL;

    location** locations = malloc(*loc_count * sizeof(location*));
    if (!locations) return NULL;

    for (int i = 0; i < *loc_count; i++) {
        locations[i] = malloc(sizeof(location));
        if (!locations[i]) {
            // Clean up previously allocated locations
            for (int j = 0; j < i; j++) {
                free(locations[j]);
            }
            free(locations);
            return NULL;
        }

        // Initialize the location
        locations[i]->id = i;
        locations[i]->type = LOC_EMPTY;
        locations[i]->original_type = LOC_EMPTY;
        locations[i]->exit_count = 0;
        for (int j = 0; j < MAX_EXITS_COUNT; j++) {
            locations[i]->exits_id[j] = -1;
        }
        locations[i]->description[0] = '\0';
    }

    return locations;
}

// Destroy a map of locations
void destroy_map(location** locs, int count) {
    if (!locs) return;

    for (int i = 0; i < count; i++) {
        if (locs[i]) {
            free(locs[i]);
        }
    }
    free(locs);
}