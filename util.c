/* util.c */

#include "util.h"

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int last_index_of(const char * s, char target)
{
   int ret = -1;
   int curIdx = 0;
   while(s[curIdx] != '\0')
   {
      if (s[curIdx] == target) ret = curIdx;
      curIdx++;
   }
   return ret;
}
void get_app_path(const char* app, char *appdir)
{	   
    char* fullpath = realpath(app, 0);
    //printf("fullpath=%s\n", fullpath);
    int index = last_index_of(fullpath, '/');
	//printf("index=%d\n", index);
	strncpy(appdir, fullpath, index);    
    free(fullpath);
}

int file_exists(const char *fname) 
{
    return (access( fname, F_OK ) != -1) ? 1 : 0;
}
void strtrim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) 
    str++;

  if(*str == 0)  // All spaces?
    return;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end))
    end--;

  // Write new null terminator character
  end[1] = '\0';
}

void write_log(const char *fmt, ...) 
{
    char message[1000];
    va_list args;
    va_start(args, fmt);
    vsprintf(message, fmt, args);
    va_end(args);
    FILE *f = fopen("c:/temp/sshfs.log", "a");
    if (f != NULL)
        fprintf(f, "sshfs-win: debug: %s\n", message);
    fclose(f);
}

int has_keys(const char *locuser, char* keyfile) 
{
    char* sysdrive = getenv("SYSTEMDRIVE");
    snprintf(keyfile, 256, "%s/users/%s/.ssh/id_rsa", sysdrive, locuser);
    write_log("trying key authentication...");
    if (!file_exists(keyfile))
    {
        write_log("key not found: %s", keyfile);
        return 0;
    }
    return 1;
}

int jsoneq(const char *json, jsmntok_t *tok, const char *s) 
{
    return (tok->type == JSMN_STRING 
    	&& (int) strlen(s) == tok->end - tok->start 
    	&& strncmp(json + tok->start, s, tok->end - tok->start) == 0) ? 0 : -1;
}

int load_json(const char* jsonfile, json_t* json)
{
    if(!file_exists(jsonfile)) {
        write_log("cannot read json file: %s", jsonfile);
        return 0;
    }
    char* JSON_STRING = 0;
    size_t size = 0;
    FILE *fp = fopen(jsonfile, "r");
    fseek(fp, 0, SEEK_END); /* Go to end of file */
    size = ftell(fp); /* How many bytes did we pass ? */
    rewind(fp);
    JSON_STRING = malloc((size + 1) * sizeof(*JSON_STRING)); /* size + 1 byte for the \0 */
    fread(JSON_STRING, size, 1, fp); /* Read 1 chunk of size bytes from fp into buffer */
    JSON_STRING[size] = '\0';
    fclose(fp);

    int i, r;
    jsmn_parser p;
    jsmntok_t t[1024]; /* We expect no more than 128 tokens */
    jsmn_init(&p);
    r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
    if (r < 0) {
        printf("Failed to parse JSON: %d\n", r);
        return 1;
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        printf("Object expected\n");
        printf("json type=%s\n", t[0].type);
        return 1;
    }
    /* Loop over all keys of the root object */
    for (i = 1; i < r; i++) {
         if (jsoneq(JSON_STRING, &t[i], "hosts") == 0) {
            int j;
            if (t[i+1].type != JSMN_ARRAY)
                continue; 
            for (j = 0; j < t[i+1].size; j++) {
                jsmntok_t *g = &t[i+j+2];
                json->hosts[j] = strndup(JSON_STRING + g->start, g->end - g->start);               
            }
            i += t[i+1].size + 1;
        }  else if (jsoneq(JSON_STRING, &t[i], "port") == 0) {
            json->port = strndup(JSON_STRING + t[i+1].start, t[i+1].end-t[i+1].start);  
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "drive") == 0) {
            json->drive = strndup(JSON_STRING + t[i+1].start, t[i+1].end-t[i+1].start);  
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "path") == 0) {
            json->path = strndup(JSON_STRING + t[i+1].start, t[i+1].end-t[i+1].start);  
            i++;
        } else {
            // ignore
            //printf("Unexpected key: %.*s\n", t[i].end-t[i].start, JSON_STRING + t[i].start);
        }
    }
    free(JSON_STRING);
    printf("Hosts:\n");
    i=0;
    while(json->hosts[i])
        printf("  - %s\n", json->hosts[i++]);
    printf("Port: %s\n",json->port);
    printf("Drive: %s\n",json->drive);
    printf("Path: %s\n",json->path);
    return 0;
}

void load_ini(const char* appdir, json_t *json)
{
    //printf("loading ini..., json.jsonfile=%s\n",json->jsonfile);
    char key[100] = {'\0'};
    char val[FILENAME_MAX] = {'\0'};
    char line[300] = {'\0'};
    size_t span = 0;
    char ini_path[FILENAME_MAX];
    snprintf(ini_path, sizeof ini_path, "%s/sshfs-win.ini", appdir);
    //printf("ini path=%s\n", ini_path);
    FILE *ini_file = fopen(ini_path, "r");
    if (!ini_file) {
    	write_log("cannot read ini file: %s", ini_path);
    } else {
	   	while (fgets (line, sizeof (line), ini_file)) {
	        char *equal = strpbrk (line, "="); //find the equal
	        if (equal) {
	            span = equal - line;
	            memcpy (key, line, span);
	            key[span] = '\0';
                strtrim(key);
	            if(strcmp(key, "json") == 0) {
	                equal++; //advance past the =
	                char *nl = strpbrk (equal, "\n"); //fine the newline
                    if (nl) {
	                    span = nl - equal;
                        //printf("span=%d\n", span);
	                    strncpy(val, equal, span);
	                    strtrim(val);
                        json->jsonfile = strdup(val);
                    } else {
                        //printf("no nl\n");
                    }
	            }
	        }
	    }
	}
    fclose(ini_file);
}