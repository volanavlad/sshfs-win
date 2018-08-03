#include <stdarg.h>
#include <stdio.h>
//#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>

#define SSHFS_ARGS                      \
    "-f",                               \
    "-opassword_stdin",                 \
    "-opassword_stdout",                \
    "-orellinks",                       \
    "-ofstypename=SSHFS",               \
    "-oStrictHostKeyChecking=no"

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

void write_log(const char *fmt, ...) {
    char message[1000];
    va_list args;
    va_start(args, fmt);
    vsprintf(message, fmt, args);
    va_end(args);
    FILE *f = fopen("c:\\temp\\sshfs.log", "a");
    if (f != NULL)
        fprintf(f, "sshfs-win: debug: %s\n", message);
    fclose(f);
}

int main(int argc, char *argv[])
{
    static const char *sshfs = "/bin/sshfs.exe";
    static const char *environ[] = { "PATH=/bin", 0 };
    struct passwd *passwd;
    char idmap[64], volpfx[256], volname[256], portopt[256], remote[256];
    char *locuser, *remuser, *locuser_dom, *host, *port, *path, *drive, *p, *envuser;

    if (3 > argc || argc > 4)
        return 2;

    envuser = getenv("USERNAME");
    write_log("USERNAME: %s", envuser);
    if (argc < 4)
        write_log("Starting %s %s %s", argv[0], argv[1], argv[2]);
    else
        write_log("Starting %s %s %s %s", argv[0], argv[1], argv[2], argv[3]);

    /* parse argv[1] */
    /* translate backslash to forward slash */
    for (p = argv[1]; *p; p++)
        if ('\\' == *p)
            *p = '/';

    /* skip class name (\\sshfs\) */
    p = argv[1];
    while ('/' == *p)
        p++;
    while (*p && '/' != *p)
        p++;
    while ('/' == *p)
        p++;

    /* parse instance name (syntax: [locuser=]remuser@host!port/path) */
    locuser = remuser = locuser_dom = 0;
    host = p;
    port = "22";
    while (*p && '/' != *p)
    {
        if ('=' == *p)
        {
            *p = '\0';
            locuser = host;
            host = p + 1;
        }
        else if ('@' == *p)
        {
            *p = '\0';
            remuser = host;
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
        if (3 == argc)
        {
            locuser = (0 == remuser) ? envuser : remuser;
        }
        else 
        {
            /* translate backslash to '+' */
            locuser_dom = argv[3];
            locuser = locuser_dom;
            for (p = locuser_dom; *p; p++)
            {
                if ('\\' == *p)
                {
                    *p = '+';
                    locuser = p + 1;
                }
            }
        }        
    }

    if (0 == remuser)
        remuser = locuser;
   
    write_log("LOCAL USER: %s", locuser);
    if (locuser == remuser && locuser == 0)
        write_log("error: invalid user");

    /* parse argv[2] */    
    drive = argv[2];

    snprintf(portopt, sizeof portopt, "-oPort=%s", port);
    snprintf(remote, sizeof remote, "%s@%s:%s", remuser, host, path);
    snprintf(volpfx, sizeof volpfx, "-oVolumePrefix=/sshfs/%s@%s/%s", remuser, host, path);
    snprintf(volname, sizeof volname, "-ovolname=/sshfs/%s@%s/%s", remuser, host, path);
    snprintf(idmap, sizeof idmap, "-ouid=-1,gid=-1");

    // I don't know the reason for getting the local uid/gid
    // as ssh needs the uid/gid from the remote user
    // commenting for now...
    // if (0 != locuser)
    // {
    //     /* get uid/gid from local user name */
    //     passwd = getpwnam(locuser);
    //     if (0 == passwd && 0 != locuser_nodom)
    //         passwd = getpwnam(locuser_nodom);
    //     if (0 != passwd)
    //         snprintf(idmap, sizeof idmap, "-ouid=%d,gid=%d", passwd->pw_uid, passwd->pw_gid);
    // }

    
    write_log("%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s ", 
        sshfs, SSHFS_ARGS, idmap, volpfx, volname, portopt, remote, drive, (void *)0, environ);

    execle(sshfs, sshfs, SSHFS_ARGS, idmap, volpfx, volname, portopt, remote, drive, (void *)0, environ);

    return 1;
}
