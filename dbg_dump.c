#include <stdio.h>
#include "popen_plus.h"

extern const char* __progname;

/* tab+name+blank */
static int max_assign_len = 0;

/* state machine states */
enum
{
    S_GET_ASSIGN = 0,
    S_GET_VALUE,
    S_GET_NAME,
    S_GET_ARRAY,
    S_LEFT_BRACE,
    S_MAX
};

/* token input type */
enum
{
    T_VALUE = 0,
    T_NAME,
    T_ASSIGN,
    T_LEFT_BRACE,
    T_RIGHT_BRACE,
    T_S_MAX,
    T_COMMA,
};

/* state transaction table */
static int state_table[S_MAX][T_S_MAX] =
{
    /* state           VALUE         NAME          ASSIGN       LEFT_B        RIGHT_B    */
    [S_GET_ASSIGN] = { -1,           -1,           S_GET_VALUE, -1,           -1         },
    [S_GET_VALUE ] = { S_GET_NAME,   -1,           -1,          S_LEFT_BRACE, -1         },
    [S_GET_NAME  ] = { -1,           S_GET_ASSIGN, -1,          S_LEFT_BRACE, S_GET_NAME },
    [S_GET_ARRAY ] = { S_GET_ARRAY,  S_GET_ARRAY,  -1,          -1,           S_GET_NAME },
    [S_LEFT_BRACE] = { S_GET_ARRAY,  S_GET_ASSIGN, -1,          S_LEFT_BRACE, -1         },
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
    printf("\n");
    print_blanks(len);
    printf(".%s ", name);
    len += strlen(name) + 2;

    if (len > max_assign_len)
    {
        max_assign_len = len;
    }

    print_blanks(max_assign_len - len);
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
            if (ret != -1)
            {
                is_done = 1;
            }
            break;

        case EOF:
            is_done = 1;
            break;

        case ',':
            if (ret == -1)
            {
                return T_COMMA;
            }

        case '{':
            if (ret == -1)
            {
                return T_LEFT_BRACE;
            }

        case '}':
            if (ret == -1)
            {
                return T_RIGHT_BRACE;
            }

        case '=':
            if (ret == -1)
            {
                return T_ASSIGN;
            }
            ungetc(c, fp);
            is_done = 1;
            break;

        default:
            *p_char++ = c;
            ret       = T_NAME;
        }
    }

    *p_char = '\0';

    if (ret == T_NAME && *token == '0')
    {
        ret = T_VALUE;
    }

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
    int  state       = S_GET_VALUE;
    int  brace_level = 0;
    int  token_ret;
    char token[256];

    /* find the first '=' */
    while ((token_ret = getc(fp)) != '=' &&
           token_ret != EOF)
    {
    }

    int is_done = 0;
    while (!is_done)
    {
        token_ret = get_token(fp, token);

        if (token_ret == -1)
        {
            break;
        }

        if (token_ret == T_COMMA)
        {
            printf(",");
            continue;
        }

        switch (state)
        {
        case -1:
            printf("state machine got an error\n");
            is_done = 1;
            break;

        case S_GET_ASSIGN:
            printf("= ");
            break;

        case S_GET_VALUE:
            if (token_ret == T_LEFT_BRACE)
            {
                brace_level++;
                printf("\n");
                print_blanks(4 * (brace_level - 1));
                printf("{");
            }
            else if (token_ret == T_VALUE)
            {
                /* print the value */
                printf("%s", token);
                if (brace_level == 0)
                {
                    is_done = 1;
                    printf("\n");
                }
            }
            break;

        case S_GET_ARRAY:
            if (token_ret == T_RIGHT_BRACE)
            {
                printf("\n");
                print_blanks(4 * (brace_level - 1));
                printf("}");
                brace_level--;
                if (brace_level == 0)
                {
                    is_done = 1;
                    printf("\n");
                }
            }
            else if (token_ret == T_VALUE || token_ret == T_NAME)
            {
                /* print the value */
                printf(" %s", token);
            }
            break;

        case S_LEFT_BRACE:
            if (token_ret == T_LEFT_BRACE)
            {
                brace_level++;
                state = S_LEFT_BRACE;
                printf("\n");
                print_blanks(4 * (brace_level - 1));
                printf("{");
            }
            else if (token_ret == T_VALUE)
            {
                /* this is the first value! */
                printf("\n");
                print_blanks(brace_level * 4);
                printf("%s", token);
            }
            else if (token_ret == T_NAME)
            {
                /* this is a name */
                print_name(token, brace_level);
            }
            break;

        case S_GET_NAME:
            if (token_ret == T_NAME)
            {
                print_name(token, brace_level);
            }
            if (token_ret == T_LEFT_BRACE)
            {
                brace_level++;
                state = S_LEFT_BRACE;
                printf("\n");
                print_blanks(4 * (brace_level - 1));
                printf("{");
            }
            else if (token_ret == T_RIGHT_BRACE)
            {
                printf("\n");
                print_blanks(4 * (brace_level - 1));
                printf("}");
                brace_level--;
                if (brace_level == 0)
                {
                    is_done = 1;
                    printf("\n");
                }
            }
            break;
        }

        state = state_table[state][token_ret];
    }
}

void dbg_dump(const char* name)
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
    fprintf(p_fp->write_fp, "p/x %s\n", name);
    fprintf(p_fp->write_fp, "p/x %s\n", name);
    fprintf(p_fp->write_fp, "detach\nquit\n");
    fflush(p_fp->write_fp);

    while (flag)
    {
	    /* trap here */
    }

    if (get_type_name(p_fp->read_fp, buf) < 0)
    {
        printf("can't get type name for: %s\n", name);
        return;
    }
    else
    {
        printf("\n(%s)(%s) = ", buf, name);
    }
    
//    int c;
//    while((c = getc(p_fp->read_fp)) != EOF) putchar(c);
//    return;

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

