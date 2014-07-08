#ifndef _DG_DBG_H_
#define _DG_DBG_H_
/*==================================================================================================

    Module Name:  dg_dbg.h

    General Description: This file provides dg_dbg interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/

/** @addtogroup libdg_dbg
@{
*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
                                              MACROS
==================================================================================================*/
#define DG_DBG_VERBOSE(x ...) DG_DBG_print(DG_DBG_LVL_VERBOSE, x)
#define DG_DBG_TRACE(x ...)   DG_DBG_print(DG_DBG_LVL_TRACE, x)
#define DG_DBG_WARN(x ...)    DG_DBG_print(DG_DBG_LVL_WARNING, x)
#define DG_DBG_ERROR(x ...)   DG_DBG_print(DG_DBG_LVL_ERROR, "ERROR: "x)

/*==================================================================================================
                                               ENUMS
==================================================================================================*/

/** debug level definition */
enum
{
    DG_DBG_LVL_VERBOSE = 0,
    DG_DBG_LVL_TRACE   = 1,
    DG_DBG_LVL_WARNING = 2,
    DG_DBG_LVL_ERROR   = 3,
    DG_DBG_LVL_DISABLE = 4,
};

/*==================================================================================================
                                   STRUCTURES AND OTHER TYPEDEFS
==================================================================================================*/

/*==================================================================================================
                                   GLOBAL VARIABLE DECLARATIONS
==================================================================================================*/

/*==================================================================================================
                                        FUNCTION PROTOTYPES
==================================================================================================*/

/*=============================================================================================*//**
@brief Sets an error string for the current thread

@param[in] format - printf style format string for error message
@param[in] ...    - Variable argument, used to populated format string

@note
- If err_string already has a string, the old one will be freed
*//*==============================================================================================*/
void DG_DBG_set_err_string(const char* format, ...);

/*=============================================================================================*//**
@brief Gets an error string for the current thread

@return - the current thread error string

@note
- get error string from thread specific data
- If no error string was set, will return NULL
- caller should NOT free the string
*//*==============================================================================================*/
char* DG_DBG_get_err_string();

/*=============================================================================================*//**
@brief Set debug level

@param[in] debug_level
*//*==============================================================================================*/
void DG_DBG_set_dbg_level(int debug_level);

/*=============================================================================================*//**
@brief Get debug level

@return current debug level
*//*==============================================================================================*/
int DG_DBG_get_dbg_level();

/*=============================================================================================*//**
@brief Log out the debug message according to the debug level

@param[in]   debug_level
@param[in]   format, .... the formated string
*//*==============================================================================================*/
void DG_DBG_print(int debug_level, const char* format, ...);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* _DG_DBG_H_ */

