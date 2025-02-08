#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>

#include "sound_service_fw.h"

class hah {
  public:
    struct sound_settings_fw *shmp = nullptr;

    void Start() {
        auto fd = shm_open(SSFW_SHMPATH, O_RDWR, 0);
        if (fd == -1)
            errExit("shm_open");

        shmp = static_cast<sound_settings_fw *>(mmap(nullptr, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (shmp == MAP_FAILED)
            errExit("mmap");
    };

    void Send() const {
        shmp->command.id = SSFW_UPDATE;
        if (sem_post(&shmp->sem1) == -1)
            errExit("sem_post");
        if (sem_wait(&shmp->sem2) == -1)
            errExit("sem_wait");
    };

    void Print() const {
        printf("asd: %s %d\n", shmp->FilterStatus[0].name, shmp->FilterStatus[0].is_proc);
        exit(0);
    };
};

int main(int argc, char *argv[]) {
    auto h = hah();
    h.Start();
    h.Send();
    h.Print();

    int fd;
    struct sound_settings_fw *shmp;

    fd = shm_open(SSFW_SHMPATH, O_RDWR, 0);
    if (fd == -1)
        errExit("shm_open");

    shmp = static_cast<sound_settings_fw *>(mmap(nullptr, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (shmp == MAP_FAILED)
        errExit("mmap");

    shmp->command.id = SSFW_UPDATE;
    if (sem_post(&shmp->sem1) == -1)
        errExit("sem_post");
    if (sem_wait(&shmp->sem2) == -1)
        errExit("sem_wait");

    printf("asd: %s %d\n", shmp->FilterStatus[0].name, shmp->FilterStatus[0].is_proc);

    exit(EXIT_SUCCESS);
}
