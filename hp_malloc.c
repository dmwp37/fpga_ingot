/*==================================================================================================

    Module Name:  hp_malloc.c

    General Description: Implements the huge page malloc

====================================================================================================

====================================================================================================
                                           INCLUDE FILES
==================================================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "rte_common.h"
#include "hp_malloc.h"

/*==================================================================================================
                                          LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
                                           LOCAL MACROS
==================================================================================================*/
#ifndef MAP_POPULATE
# define MAP_POPULATE 0 /* hack for cygwin build */
#endif

#ifndef MAP_HUGETLB
# define MAP_HUGETLB 0
#endif

#define BAD_PHYS_ADDR ((phys_addr_t)-1)

/*==================================================================================================
                            LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static phys_addr_t mem_addr_virt2phy(const void* virtaddr);

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
@brief malloc huge pages memory zone

@param[in] size - the size of the huge page memory

@return pointer to huge page structure
*//*==============================================================================================*/
hp_t* hp_alloc(size_t size)
{
    size_t real_size = RTE_ALIGN(size, HUGE_PASE_SIZE);
    hp_t*  p_hp      = malloc(sizeof(hp_t));

    if (p_hp == NULL)
    {
        return NULL;
    }

    void* ptr = mmap(NULL, real_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE | MAP_HUGETLB, -1, 0);
    if (ptr == MAP_FAILED)
    {
        printf("%s(): cannot mmap huge page: errno=%d(%m)\n", __func__, errno);
        free(p_hp);
        p_hp = NULL;
    }
    else if ((p_hp->phys_addr = mem_addr_virt2phy(ptr)) == BAD_PHYS_ADDR)
    {
        hp_free(p_hp);
        p_hp = NULL;
    }
    else
    {
        p_hp->base = ptr;
        p_hp->size = real_size;
        /* assert if physical address is aligned */
        if ((p_hp->phys_addr & 0xFFF) != 0)
        {
            printf("%s(): physical address not aligned!\n", __func__);
            exit(1);
        }
    }

    return p_hp;
}

/*=============================================================================================*//**
@brief free huge pages memory zone

@param[in] p_hp - pointer to huge page structure
*//*==============================================================================================*/
void hp_free(hp_t* p_hp)
{
    if (p_hp == NULL)
    {
        return;
    }

    munmap(p_hp->base, p_hp->size);
    free(p_hp);
}

/*==================================================================================================
                                          LOCAL FUNCTIONS
==================================================================================================*/

/*=============================================================================================*//**
@brief Get physical address of any mapped virtual address in the current process.

@param[in] virtaddr     - the virtual address

@return the physical address of the virtual address
*//*==============================================================================================*/
phys_addr_t mem_addr_virt2phy(const void* virtaddr)
{
    int           fd;
    uint64_t      page, physaddr;
    unsigned long virt_pfn;
    int           page_size;

    /* standard page size */
    page_size = getpagesize();

    fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd < 0)
    {
        printf("%s(): cannot open /proc/self/pagemap: errno=%d(%m)\n", __func__, errno);
        return BAD_PHYS_ADDR;
    }

    off_t offset;
    virt_pfn = (unsigned long)virtaddr / page_size;
    offset   = sizeof(uint64_t) * virt_pfn;
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
    {
        printf("%s(): seek error in /proc/self/pagemap: errno=%d(%m)\n", __func__, errno);
        close(fd);
        return BAD_PHYS_ADDR;
    }
    if (read(fd, &page, sizeof(uint64_t)) < 0)
    {
        printf("%s(): cannot read /proc/self/pagemap: errno=%d(%m)\n", __func__, errno);
        close(fd);
        return BAD_PHYS_ADDR;
    }

    /*
     * the pfn (page frame number) are bits 0-54 (see
     * pagemap.txt in linux Documentation)
     */
    physaddr = ((page & 0x7fffffffffffffULL) * page_size) +
               ((unsigned long)virtaddr % page_size);
    close(fd);
    return physaddr;
}

