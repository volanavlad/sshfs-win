#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdio.h>
#include "jsmn.h"


#define MAX_HOSTS 1000

typedef struct {
    char *hosts[MAX_HOSTS];
    char *port;
    char *drive;
    char *path;
    char *jsonfile;
} json_t;

int last_index_of(const char *s, char target);
void strtrim(char *str);
void get_app_path(const char *app, char *dir);

/* check if file exists */
int file_exists(const char *fname);

/* logging */
void write_log(const char *fmt, ...);

/* get ssh key file */
int has_keys(const char *locuser, char* keyfile);

/* json */
int jsoneq(const char *json, jsmntok_t *tok, const char *s);
int load_json(const char* jsonfile, json_t* json);
void load_ini(const char* appdir, json_t* json);

#endif /* __UTIL_H_ */