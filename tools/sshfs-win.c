#include <stdarg.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn.h"

#define SSHFS_ARGS                      \
    "-f",                               \
    "-orellinks",                       \
    "-ofstypename=SSHFS",               \
    "-oStrictHostKeyChecking=no",       \
    "-d",                               \
    "-odebug",                          \
    "-osshfs_debug",                    \
    "-oLOGLEVEL=DEBUG3"
    //"-oUserKnownHostsFile=/dev/null",   \
    //"-opassword_stdin",                 \
    //"-opassword_stdout",                \
    
#if 0
#define execle pr_execl
static void pr_execl(const char *path, ...)
{
    va_list ap;
    const char *arg;

    va_start(ap, path);
    fprintf(stderr, "%s\n", path);
    while (0 != (arg = va_arg(ap, const char *)))
        fprintf(stderr, "    %s\n", arg);
    va_end(ap);
}
#endif


char g_logfile[256] = {0};
char g_jsonfile[256] = {0};
char g_keyfile[256] = {0};
char g_configfile[256] = {0};

#define MAX_HOSTS 1000

typedef struct {
    char* hosts[MAX_HOSTS];
    int port;
    char* drive;
    char* path;
} json_t;

json_t g_json;

void forward_slashes(char* string) {
    char *p;
    for (p = string; *p; p++)
        if ('\\' == *p)
            *p = '/';
}

void init() {
    char *home = getenv("USERPROFILE");

    snprintf(g_logfile, sizeof g_logfile, "%s\\.ssh\\sshfs.log", home);
    snprintf(g_jsonfile, sizeof g_jsonfile, "%s\\.ssh\\sshfs.json", home);
    snprintf(g_keyfile, sizeof g_keyfile, "%s\\.ssh\\id_rsa", home);
    snprintf(g_configfile, sizeof g_configfile, "%s\\.ssh\\sshfs.config", home);
    printf("log  file=%s\n", g_logfile);
    printf("json file=%s\n", g_jsonfile);
    printf("key  file=%s\n", g_keyfile);
    printf("conf file=%s\n", g_configfile);

    for (int i = 0; i < MAX_HOSTS; ++i) {
        g_json.hosts[i] = malloc(30);
        g_json.hosts[i] = '\0';
    }
    g_json.port = 22;
    g_json.drive = malloc(2);
    g_json.path = malloc(256);
}

int file_exists(const char *fname) {
    /* check if file exists */
    return access( fname, F_OK ) != -1;
}

void write_log(const char *text) {
    printf("log: %s\n", text);
    FILE *f = fopen(g_logfile, "a");
    if (f != NULL)
        fprintf(f, "log: %s\n", text);
    fclose(f);
}

char* load_json(const char* fname) {
    if(!file_exists(fname)) {
        printf("cannot read config file: %s\n", fname);
        return 0;
    }
    char* buffer = 0;
    size_t size = 0;
    FILE *fp = fopen(fname, "r");
    fseek(fp, 0, SEEK_END); /* Go to end of file */
    size = ftell(fp); /* How many bytes did we pass ? */
    rewind(fp);
    buffer = malloc((size + 1) * sizeof(*buffer)); /* size + 1 byte for the \0 */
    fread(buffer, size, 1, fp); /* Read 1 chunk of size bytes from fp into buffer */
    buffer[size] = '\0';
    // printf("%s\n", buffer);
    return buffer;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

int parse_json(const char* jsonfile) {
    int i;
    int r;
    char* JSON_STRING = load_json(jsonfile);
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
                g_json.hosts[j] = strndup(JSON_STRING + g->start, g->end - g->start);               
            }
            i += t[i+1].size + 1;
        }  else if (jsoneq(JSON_STRING, &t[i], "port") == 0) {
            char* sport = strndup(JSON_STRING + t[i+1].start, t[i+1].end-t[i+1].start);  
            g_json.port = atoi(sport);             
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "drive") == 0) {
            //printf("- Drive: %.*s\n", t[i+1].end-t[i+1].start, JSON_STRING + t[i+1].start);
            g_json.drive = strndup(JSON_STRING + t[i+1].start, t[i+1].end-t[i+1].start);  
            i++;
        } else if (jsoneq(JSON_STRING, &t[i], "path") == 0) {
            //printf("- Path: %.*s\n", t[i+1].end-t[i+1].start, JSON_STRING + t[i+1].start);
            g_json.path = strndup(JSON_STRING + t[i+1].start, t[i+1].end-t[i+1].start);  
            i++;
        } else {
            // ignore
            //printf("Unexpected key: %.*s\n", t[i].end-t[i].start, JSON_STRING + t[i].start);
        }
    }
    free(JSON_STRING);
    printf("Hosts:\n");
    i=0;
    while(g_json.hosts[i])
        printf("  - %s\n", g_json.hosts[i++]);
    printf("Port: %d\n",g_json.port);
    printf("Drive: %s\n",g_json.drive);
    printf("Path: %s\n",g_json.path);
    return 0;
}

int main(int argc, char *argv[])
{
    write_log("starting sshfs-win.exe...");
    init();
    parse_json(g_jsonfile);
    
    static const char *sshfs = "/bin/sshfs.exe";
    static const char *environ[] = { "PATH=/bin", 0 };
    struct passwd *passwd;
    char idmap[64], volpfx[256], portopt[256], remote[256];
    char *locuser, *locuser_nodom, *remote_user, *userhost, *port, *path, *p, *classname, *host;

    if (3 > argc || argc > 4)
        return 2;

    /* translate backslash to forward slash */
    for (p = argv[1]; *p; p++)
        if ('\\' == *p)
            *p = '/';

    //printf("argc=%d\n", argc);
    char *puser = getenv("USERNAME");
    if (!puser)
        puser = getenv("USER");
    //printf("user env=%s\n", puser);
    
    /* get class name (\\sshfs\) */
    p = argv[1];
    while ('/' == *p)
        p++;
    classname = p;
    while (*p && '/' != *p)
        p++;
    if (*p)
        *p++ = '\0';
    //printf("class name=%s\n", classname);
    //printf("remaining p=%s\n", p);
    
    /* parse instance name (syntax: [locuser=]user@host!port) */
    locuser = locuser_nodom = remote_user = 0;
    userhost = host = p;
    port = "22";
    while (*p && '/' != *p)
    {
        if ('=' == *p)
        {
            *p = '\0';
            locuser = userhost;
            userhost = p + 1;
            host = userhost;
        }
        else if ('@' == *p)
        {
            *p = '\0';
            remote_user = userhost;
            host = p + 1;
        }
        else if ('!' == *p)
        {
            *p = '\0';
            port = p + 1;
        }
        p++;
    }
    if (*p)
        *p++ = '\0';
    path = p;


    /* get local user name */
    if (0 == locuser)
    {
        if (3 >= argc)
        {
            p = userhost;
            while (*p && '@' != *p)
                p++;
            if (*p)
            {
                *p = '\0';
                locuser = userhost;
            }   
        }
        else
        {
            /* translate backslash to '+' */
            locuser = argv[3];
            for (p = locuser; *p; p++)
                if ('\\' == *p)
                {
                    *p = '+';
                    locuser_nodom = p + 1;
                }
        }
    }

    if (0 == locuser)
        locuser = puser;
    
    snprintf(idmap, sizeof idmap, "-ouid=-1,gid=-1");
    // if (0 != locuser)
    // {
    //     /* get uid/gid from local user name */
    //     passwd = getpwnam(locuser);
    //     if (0 == passwd && 0 != locuser_nodom)
    //         passwd = getpwnam(locuser_nodom);
    //     if (0 != passwd)
    //         snprintf(idmap, sizeof idmap, "-ouid=%d,gid=%d", passwd->pw_uid, passwd->pw_gid);
    // }
    if (0 != locuser_nodom)
        locuser = locuser_nodom;
    if (0 == remote_user)
        remote_user = locuser;
   
    snprintf(remote, sizeof remote, "%s@%s:%s", remote_user, host, path);
    snprintf(volpfx, sizeof volpfx, "-oVolumePrefix=/%s/%s@%s", classname, remote_user, host);
    snprintf(portopt, sizeof portopt, "-oPort=%s", port);

    // char* q;
    // printf("remote=%s, len=%d\n",remote , strlen(remote));
    // for (q = remote; *q; q++)
    //     printf("%s\n", q);

    // printf("locuser=%s\n", locuser);
    // printf("locuser_nodom=%s\n", locuser_nodom);
    // printf("remote_user=%s\n", remote_user);
    char pass[256];
    if (file_exists(g_keyfile))
        snprintf(pass, sizeof pass, "");
    else
        snprintf(pass, sizeof pass, "-opassword_stdin -opassword_stdout");
    
    char config[256];   
    if (file_exists(g_configfile))
        //snprintf(config, sizeof config, "-F %s", g_configfile);
        snprintf(config, sizeof config, "-F c:\\Users\\sant\\.ssh\\sshfs.config");
    else
        snprintf(config, sizeof config, "");
    forward_slashes(config);
    printf("config=%s\n", config);
    // printf("remote=%s\n", remote);
    // printf("argv2=%s\n", argv[2]);
    char cmd[500];
    char cmd2[500];
    char cmd3[500];
    snprintf(cmd, sizeof cmd, "%s %s %s", sshfs, SSHFS_ARGS, pass);
    snprintf(cmd2, sizeof cmd2, "%s %s %s %s", cmd, idmap, volpfx, portopt);
    snprintf(cmd3, sizeof cmd3, "%s %s %s %s", cmd2, config, remote, argv[2]);
    write_log(cmd3);

    // printf("has_null %s = %d\n", remote, has_null(remote, sizeof remote) );
    // printf("has_null %s = %d\n", config, has_null(config, sizeof config) );
    // printf("has_null %s = %d\n", argv[2], has_null(argv[2], sizeof argv[2]) );
    // printf("has_null %s = %d\n", pass, has_null(pass, 20) );


    execle(sshfs, sshfs, SSHFS_ARGS, pass, idmap, volpfx, portopt, config, remote, argv[2], (void *)0, environ);

    return 1;
}
