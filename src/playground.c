#include <stdio.h>
#include <stdlib.h>
// for pid
#include <sys/types.h>

void rotate_fds(pid_t *fds, int count)
{
    if (count > 1)
    {
        int first = fds[0];
        for (int i = 0; i < count - 1; ++i)
        {
            fds[i] = fds[i + 1];
        }
        fds[count - 1] = first;
    }
}

#define NUM_COMPONENTS 3

int main()
{

    pid_t fds[3] = {1, 2, 3};
    rotate_fds(fds, NUM_COMPONENTS);
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        printf("%d\n", fds[i]);
    }
    rotate_fds(fds, NUM_COMPONENTS);
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        printf("%d\n", fds[i]);
    }
    rotate_fds(fds, NUM_COMPONENTS);
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        printf("%d\n", fds[i]);
    }
    return 0;
}
