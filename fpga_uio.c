/*==================================================================================================

    Module Name:  fpga_uio.c

    General Description: Implements the FPGA uio interface

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "fpga_uio.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/
#define FPGA_INGOT_DRIVER "Ingot FPGA UIO"

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
typedef struct
{
    char  name[24];
    int   fd;
    int   mmap_size;
    void* base;
} FPGA_UIO_T;

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static int  uio_get_mem_size(const char* uio, int map_num);
static bool is_expected_uio(const char* name, const char* driver);
static bool uio_find(const char* driver, char* uio_name);

/*==================================================================================================
                                         GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                                          LOCAL VARIABLES
==================================================================================================*/
static FPGA_UIO_T fpga_uio =
{
    .fd        = -1,
    .base      = NULL,
    .mmap_size = -1,
};

/*==================================================================================================
                                         GLOBAL FUNCTIONS
==================================================================================================*/
/*=============================================================================================*//**
@brief Init FPGA UIO

@return true for success
*//*==============================================================================================*/
bool FPGA_UIO_init()
{
    char  fpga_dev[128];
    int   fpga_fd;
    void* fpga_base = NULL;
    int   mmap_size = -1;


    if (fpga_uio.base != NULL)
    {
        /* already opened */
        return true;
    }

    if (!uio_find(FPGA_INGOT_DRIVER, fpga_uio.name))
    {
        printf("error: Can't find FPGA device\n");
        return false;
    }

    snprintf(fpga_dev, sizeof(fpga_dev), "/dev/%s", fpga_uio.name);

    if ((fpga_fd = open(fpga_dev, O_RDWR | O_SYNC)) < 0)
    {
        printf("error: Failed to open FPGA device %s. errno=%d(%m)\n", fpga_dev, errno);
        return false;
    }
    else
    {
        fpga_uio.fd = fpga_fd;
    }

    if ((mmap_size = uio_get_mem_size(fpga_uio.name, 0)) < 0)
    {
        printf("error: can't get fpag uio mmap0 size\n");
        goto init_err;
    }
    else
    {
        fpga_uio.mmap_size = mmap_size;
    }

    if ((fpga_base = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, fpga_uio.fd, 0)) == MAP_FAILED)
    {
        printf("error: Failed to mmap. errno=%d(%m)\n", errno);
        goto init_err;
    }
    else
    {
        fpga_uio.base = fpga_base;
    }

    return true;

init_err:
    if (fpga_uio.fd > 0)
    {
        close(fpga_uio.fd);
        fpga_uio.fd = -1;
    }
    return false;
}

/*=============================================================================================*//**
@brief Exit FPGA UIO

*//*==============================================================================================*/
void FPGA_UIO_exit()
{
    if (fpga_uio.base != NULL)
    {
        munmap(fpga_uio.base, fpga_uio.mmap_size);
        fpga_uio.mmap_size = -1;
        fpga_uio.base      = NULL;
    }

    if (fpga_uio.fd > 0)
    {
        close(fpga_uio.fd);
        fpga_uio.fd = -1;
    }
}

/*=============================================================================================*//**
@brief Get the FPGA register

*//*==============================================================================================*/
void* FPGA_UIO_get_base()
{
    return fpga_uio.base;
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
            else if (is_expected_uio(namelist[i]->d_name, driver))
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
bool is_expected_uio(const char* name, const char* driver)
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

