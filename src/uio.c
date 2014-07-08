/*==================================================================================================

    Module Name:  uio.c

    General Description: Implements the uio interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "uio.h"

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
static int  uio_get_mem_size(const char* uio, int map_num);
static bool uio_is_expected(const char* name, const char* driver);
static bool uio_find(const char* driver, char* uio_name);

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/
/*=============================================================================================*//**
@brief Init UIO driver

@return the uio structure
*//*==============================================================================================*/
uio_t* uio_init(const char* driver_name)
{
    char   dev_path[128];
    int    fd;
    void*  mmap_base = NULL;
    int    mmap_size = -1;
    uio_t* uio       = malloc(sizeof(uio_t));

    if (uio == NULL)
    {
        return NULL;
    }

    if (!uio_find(driver_name, uio->name))
    {
        printf("error: Can't find UIO device for %s\n", driver_name);
        free(uio);
        return NULL;
    }

    snprintf(dev_path, sizeof(dev_path), "/dev/%s", uio->name);

    if ((fd = open(dev_path, O_RDWR | O_SYNC)) < 0)
    {
        printf("error: Failed to open uio device %s. errno=%d(%m)\n", dev_path, errno);
        free(uio);
        return NULL;
    }
    uio->fd = fd;

    if ((mmap_size = uio_get_mem_size(uio->name, 0)) < 0)
    {
        printf("error: can't get %s mmap0 size\n", uio->name);
        goto init_err;
    }
    uio->mmap_size = mmap_size;

    if ((mmap_base = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, uio->fd, 0)) == MAP_FAILED)
    {
        printf("error: Failed to mmap. errno=%d(%m)\n", errno);
        goto init_err;
    }
    uio->base = mmap_base;

    return uio;

init_err:
    if (uio->fd > 0)
    {
        close(uio->fd);
        uio->fd = -1;
    }
    return NULL;
}

/*=============================================================================================*//**
@brief release UIO driver

*//*==============================================================================================*/
void uio_exit(uio_t* uio)
{
    if (uio == NULL)
    {
        return;
    }

    if (uio->base != NULL)
    {
        munmap(uio->base, uio->mmap_size);
        uio->mmap_size = -1;
        uio->base      = NULL;
    }

    if (uio->fd > 0)
    {
        close(uio->fd);
        uio->fd = -1;
    }

    free(uio);
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief get the uio mmap size

@param[in] uio     - the uio sysfs entry
@param[in] map_num - index of the map

@return the mmap size

*//*==============================================================================================*/
int uio_get_mem_size(const char* uio, int map_num)
{
    int   ret = -1;
    char  filename[128];
    FILE* file;

    sprintf(filename, "/sys/class/uio/%s/maps/map%d/size", uio, map_num);

    if ((file = fopen(filename, "r")) == NULL)
    {
        printf("error: can't open %s. errno=%d(%m)\n", filename, errno);
    }
    else if (fscanf(file, "0x%x", &ret) != 1)
    {
        printf("error: fscanf %s failed. errno=%d(%m)\n", filename, errno);
    }
    else
    {
        printf("%s mmap %d size is 0x%x\n", uio, map_num, ret);
    }

    if (file != NULL)
    {
        fclose(file);
    }

    return ret;
}

/*=============================================================================================*//**
@brief find the UIO driver device

@param[in]  driver   - The expected driver name of the uio
@param[out] uio_name - The name of the uio

@return TURE if successfully find the uio driver

*//*==============================================================================================*/
bool uio_find(const char* driver, char* uio_name)
{
    bool            is_find = false;
    int             n;
    struct dirent** namelist = NULL;


    if ((n = scandir("/sys/class/uio", &namelist, NULL, alphasort)) < 0)
    {
        printf("error: can't scan /sys/class/uio. errno=%d(%m)\n", errno);
    }
    else
    {
        int i;

        for (i = 0; i < n; i++)
        {
            if (is_find ||
                (strcmp(namelist[i]->d_name, ".") == 0) ||
                (strcmp(namelist[i]->d_name, "..") == 0))
            {
                /* do nothing */
            }
            else if (uio_is_expected(namelist[i]->d_name, driver))
            {
                strcpy(uio_name, namelist[i]->d_name);
                printf("uio_name is '%s' for driver '%s'\n", uio_name, driver);
                is_find = true;
            }

            free(namelist[i]);
        }
        free(namelist);
    }

    return is_find;
}

/*=============================================================================================*//**
@brief judge if the uio is expected driver

@param[in] name   - The uio entry under /sys/class/uio
@param[in] driver - The expected driver name of the uio

@return TRUE if the uio is expected, otherwise return FALSE

*//*==============================================================================================*/
bool uio_is_expected(const char* name, const char* driver)
{
    bool  ret = false;
    char  filename[128];
    FILE* fp = NULL;

    snprintf(filename, sizeof(filename), "/sys/class/uio/%s/name", name);

    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("can not open %s. errno=%d(%m)\n", filename, errno);
    }
    else
    {
        char*  line = NULL;
        size_t len;

        if (getline(&line, &len, fp) != -1)
        {
            /* see if the name contains our driver name */
            if (strstr(line, driver) != NULL)
            {
                printf("/sys/class/uio/%s is expected for %s\n", name, driver);
                ret = true;
            }
        }

        free(line);
        fclose(fp);
    }

    return ret;
}

