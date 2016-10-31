#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/wait.h>

#include "vsd_device.h"

#define TEST(assertion) \
    if (!(assertion)) { \
        fprintf(stderr, "[%5d] %s:%d\n", getpid(), __FILE__, __LINE__); \
        perror("Test failed"); \
        abort(); \
    }

void pipe_notify(int pipefd[2]) {
    char buf = 0;
    TEST(write(pipefd[1], &buf, 1) == 1);
}

void pipe_wait(int pipefd[2]) {
    char buf;
    TEST(read(pipefd[0], &buf, 1) == 1);
}

int main()
{
    TEST(!vsd_init());

    const size_t PAGE_SIZE = getpagesize();
    const size_t vsd_offset = PAGE_SIZE;
    size_t vsd_size = 0;
    TEST(!vsd_get_size(&vsd_size));
    vsd_size -= vsd_offset;

    char *vsd_rw_buf = malloc(vsd_size);
    size_t i = 0;
    for (; i < vsd_size; ++i) {
        vsd_rw_buf[i] = i % 255;
    }

    TEST(vsd_write(vsd_rw_buf, vsd_offset, vsd_size) >= 0);

    char* vsd_mem = vsd_mmap(vsd_offset);
    TEST(vsd_mem);

    // Check that written data is visible by mmap
    TEST(!memcmp(vsd_rw_buf, vsd_mem, vsd_size));
    // Check in opposite direction
    vsd_mem[10] = ~vsd_mem[10];
    TEST(vsd_read(vsd_rw_buf, vsd_offset, vsd_size) >= 0);
    TEST(!memcmp(vsd_rw_buf, vsd_mem, vsd_size));

    // While we are mapped vsd, its size can't be changed
    TEST(vsd_set_size(vsd_size / 2));

    // Test that forked mmap lives independently.
    printf("Test fork()\n");
    int c2p[2], p2c[2];
    TEST(!pipe(p2c));
    TEST(!pipe(c2p));

    pid_t child = fork();
    TEST(child >= 0);
 
    // Check that written data is visible by mmap
    TEST(!memcmp(vsd_rw_buf, vsd_mem, vsd_size));

    // Cannot change size, because it's mapped twice.
    if (child == 0) {
        // Inside child
        // Step 1
        printf("Child tries to vsd_set_size\n");
        TEST(vsd_set_size(vsd_size / 2));
	pipe_notify(c2p);

        // Step 2
	pipe_wait(p2c);

        // Step 3
        printf("Child unmaps\n");
        TEST(!vsd_munmap(vsd_mem, vsd_offset));
        TEST(vsd_set_size(vsd_size / 2));
	pipe_notify(c2p);

        // Step 4
	pipe_wait(p2c);

        // Step 5
        printf("Child tries to vsd_set_size\n");
        TEST(!vsd_set_size(vsd_size / 2));
        exit(0);
    } else {
        // Inside parent
        // Step 1
	pipe_wait(c2p);

        // Step 2
        printf("Parent tries to vsd_set_size\n");
        TEST(vsd_set_size(vsd_size / 2));
	pipe_notify(p2c);

        // Step 3
	pipe_wait(c2p);

        // Step 4
        printf("Parent unmaps\n");
        TEST(!vsd_munmap(vsd_mem, vsd_offset));
        TEST(!vsd_set_size(vsd_size / 2));
	pipe_notify(p2c);

        // Step 5
        int status;
        waitpid(child, &status, 0);
        TEST(WIFEXITED(status));
        TEST(WEXITSTATUS(status) == 0);
    }

    // Now ok
    TEST(!vsd_set_size(vsd_size / 2));
    TEST(!vsd_set_size(vsd_size));

    free(vsd_rw_buf);

    TEST(!vsd_deinit());
    printf("Ok\n");
    return 0;
}
