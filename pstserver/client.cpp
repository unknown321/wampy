#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>

#include "pshm_ucase.h"

int main(int argc, char *argv[]) {
    int fd;
    struct sound_settings *shmp;

    /* Open the existing shared memory object and map it
       into the caller's address space. */

    fd = shm_open(SHMPATH, O_RDWR, 0);
    if (fd == -1)
        errExit("shm_open");

    shmp = static_cast<sound_settings *>(mmap(nullptr, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (shmp == MAP_FAILED)
        errExit("mmap");

    /* Tell peer that it can now access shared memory. */

    if (sem_post(&shmp->sem1) == -1)
        errExit("sem_post");

    /* Wait until peer says that it has finished accessing
       the shared memory. */

    if (sem_wait(&shmp->sem2) == -1)
        errExit("sem_wait");

    /* Write modified data in shared memory to standard output. */

    shmp->Print();

    exit(EXIT_SUCCESS);
}