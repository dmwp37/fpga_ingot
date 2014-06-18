#include <stdio.h>
#include "popen_plus.h"

extern const char* __progname;

/* tab+name+blank */
static int max_assign_len = 0;

enum
{
    S_IDLE,
    S_GET_VALUE,
    S_GET_NAME,
};

static void print_blanks(int num)
{
    while (num--)
    {
        printf(" ");
    }
}

static void print_name(char* name, int level)
{
    int len = level * 4;
    print_blanks(len);
    printf(".%s ", name);
    len += strlen(name) + 2;

    if (len > max_assign_len)
    {
        max_assign_len = len;
    }

    print_blanks(max_assign_len - len);

    printf("= ");
}

static int get_token(FILE* fp, char* token)
{
    int   ret = -1; /* no token */
    int   c;
    char* p_char  = token;
    int   is_done = 0;
    while (!is_done)
    {
        c = fgetc(fp);

        switch (c)
        {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
        case ',':
            if (ret != -1)
            {
                is_done = 1;
            }
            break;

        case EOF:
            is_done = 1;
            break;

        case '{':
        case '}':
        case '=':
            if (ret == -1)
            {
                return c;
            }
            ungetc(c, fp);
            is_done = 1;
            break;

        default:
            *p_char++ = c;
            ret       = 1;
        }
    }

    *p_char = '\0';

    return ret;
}

static int get_type_name(FILE* fp, char* type_name)
{
    int         ret  = -1;
    char*       line = NULL, * p;
    size_t      len;
    const char* match_key = "type = ";

    int is_done = 0;
    while (!is_done)
    {
        if (getline(&line, &len, fp) < 0)
        {
            break;
        }

        if ((p = strstr(line, match_key)) != NULL)
        {
            ret     = 1;
            is_done = 1;
            p      += strlen(match_key);
            strcpy(type_name, p);
            type_name[strlen(type_name) - 1] = '\0';
        }
    }

    free(line);

    return ret;
}

static void stat_parsor(FILE* fp)
{
    int  state       = S_IDLE;
    int  brace_level = 0;
    int  token_ret;
    char token[256];

    int is_done = 0;
    while (!is_done)
    {
        token_ret = get_token(fp, token);

        if (token_ret == -1)
        {
            break;
        }

        switch (state)
        {
        case S_IDLE:
            if (token_ret == '=')
            {
                state = S_GET_VALUE;
            }
            break;

        case S_GET_VALUE:
            if (token_ret == '{')
            {
                brace_level++;
                state = S_GET_NAME;
                printf("\n");
                print_blanks(4 * (brace_level - 1));
                printf("{\n");
            }
            else
            {
                /* print the value */
                printf("%s,\n", token);
                state = S_GET_NAME;
            }
            break;

        case S_GET_NAME:
            if (token_ret == '}')
            {
                print_blanks(4 * (brace_level - 1));
                printf("}\n");
                brace_level--;
                if (brace_level == 0)
                {
                    is_done = 1;
                }
            }
            else
            {
                print_name(token, brace_level);
                state = S_IDLE;
            }
            break;
        }
    }
}

void dump_struct(const char* name, void* address)
{
    struct popen_plus_process* p_fp;

    char buf[256];
    int  flag = 1;
    int  old_stdout_fd;

    sprintf(buf, "gdb -n -q %s %d", __progname, getpid());
    p_fp = popen_plus(buf);

    fprintf(p_fp->write_fp, "thread 1\n");
    fprintf(p_fp->write_fp, "set var flag=0\n");
    fprintf(p_fp->write_fp, "frame 1\n");
    fprintf(p_fp->write_fp, "whatis %s\n", name);
    fprintf(p_fp->write_fp, "p/x *%s\n", name);
    fprintf(p_fp->write_fp, "p/x *%s\n", name);
    fprintf(p_fp->write_fp, "detach\nquit\n");
    fflush(p_fp->write_fp);

    while (flag);  /* trap here */

    if (get_type_name(p_fp->read_fp, buf) < 0)
    {
        printf("can't get type name");
        return;
    }
    else
    {
        printf("*(%s)(%s) = /* @%p */", buf, name, address);
    }

    /* calc the max assign len */
    max_assign_len = 0;
    old_stdout_fd  = dup(fileno(stdout));
    freopen("/dev/null", "w", stdout);

    /* dump all the output to /dev/null */
    stat_parsor(p_fp->read_fp);
    fflush(stdout);

    /* set back stdout */
    dup2(old_stdout_fd, fileno(stdout));
    close(old_stdout_fd);

    /* second pass for print */
    stat_parsor(p_fp->read_fp);

    popen_plus_close(p_fp);
}

