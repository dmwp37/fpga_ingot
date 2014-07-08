/*==================================================================================================

    Module Name:  dg_dbg.c

    General Description: Implements the dg_dbg library

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <errno.h>
#include "dg_dbg.h"

/** @addtogroup libdg_dbg
@{
*/
/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static void dg_dbg_init_err_string();
static void dg_dbg_create_error_string_key();
static void dg_dbg_free_error_string(void* err_str);

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/
static pthread_key_t  dg_dbg_error_string_key;
static pthread_once_t dg_dbg_key_once = PTHREAD_ONCE_INIT;

/*default dbg level is just print error */
static int dg_dbg_level = DG_DBG_LVL_ERROR;

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief Set debug level

@param[in] debug_level
*//*==============================================================================================*/
void DG_DBG_set_dbg_level(int debug_level)
{
    if (debug_level < DG_DBG_LVL_VERBOSE)
    {
        dg_dbg_level = DG_DBG_LVL_VERBOSE;
    }
    else if (debug_level > DG_DBG_LVL_DISABLE)
    {
        dg_dbg_level = DG_DBG_LVL_DISABLE;
    }
    else
    {
        dg_dbg_level = debug_level;
    }
}

/*=============================================================================================*//**
@brief Get debug level

@return current debug level
*//*==============================================================================================*/
int DG_DBG_get_dbg_level()
{
    return dg_dbg_level;
}

/*=============================================================================================*//**
@brief Log out the debug message according to the debug level

@param[in]   debug_level
@param[in]   format, .... the formated string
*//*==============================================================================================*/
void DG_DBG_print(int debug_level, const char* format, ...)
{
    va_list args;

    va_start(args, format);
    if (debug_level >= dg_dbg_level)
    {
        printf("DIAG: ");
        vprintf(format, args);
        printf("\n");
        fflush(stdout);
    }
    va_end(args);
}

/*=============================================================================================*//**
@brief Sets an error string for the current thread

@param[in] format - printf style format string for error message
@param[in] ...    - Variable argument, used to populated format string

@note
- If err_string already has a string, the old one will be freed
*//*==============================================================================================*/
void DG_DBG_set_err_string(const char* format, ...)
{
    va_list args;       /* Variable arg list */
    char*   p_err_str;  /* pointer to the error string */

    dg_dbg_init_err_string();

    /* Init variable arg list */
    va_start(args, format);

    if (vasprintf(&p_err_str, format, args) < 0)
    {
        DG_DBG_ERROR("vasprintf() set error string failed. errno=%d(%m)", errno);
    }
    else
    {
        char* old_err_str = (char*)pthread_getspecific(dg_dbg_error_string_key);

        /* Free any existing error string */
        if (old_err_str != NULL)
        {
            DG_DBG_TRACE("Overwriting error string: %s", old_err_str);
            free(old_err_str);
        }

        /* Set new error string */
        if (pthread_setspecific(dg_dbg_error_string_key, p_err_str) != 0)
        {
            DG_DBG_ERROR("pthread_setspecific() set error string failed. errno=%d(%m)", errno);
        }
        else
        {
            DG_DBG_ERROR("Thread error string set to: %s", p_err_str);
        }
    }

    va_end(args);
}

/*=============================================================================================*//**
@brief Gets an error string for the current driver thread

@return - the current driver thread error string

@note
- get error string from thread specific data
- If no error string was set, will return NULL
- caller should NOT free the string
*//*==============================================================================================*/
char* DG_DBG_get_err_string()
{
    return pthread_getspecific(dg_dbg_error_string_key);
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief Initializes error string for the current thread

@note
- the init would only happen once for threads, please see the pthread_once manual
*//*==============================================================================================*/
void dg_dbg_init_err_string()
{
    static int b_init = 0;

    if (b_init != 0)
    {
        return;
    }

    if (pthread_once(&dg_dbg_key_once, dg_dbg_create_error_string_key) != 0)
    {
        DG_DBG_ERROR("pthread_once() for error string key failed. errno=%d(%m)", errno);
    }
    else
    {
        b_init = 1;
    }
}

/*=============================================================================================*//**
@brief Create error string key for threads specific data

@note
- this function register a free error string hanldler when thread exit
- you should free the old error string before set a new error string
*//*==============================================================================================*/
void dg_dbg_create_error_string_key()
{
    int ret = pthread_key_create(&dg_dbg_error_string_key, dg_dbg_free_error_string);

    if (ret != 0)
    {
        DG_DBG_ERROR("pthread_key_create() for error string failed. errno=%d(%m)", errno);
    }
}

/*=============================================================================================*//**
@brief Initializes error string for the current thread

@note
- when thread exit automatically call this function to free the error string
*//*==============================================================================================*/
void dg_dbg_free_error_string(void* err_str)
{
    DG_DBG_TRACE("Free error string for thread %p", pthread_self());
    free(err_str);
}


/** @} */

