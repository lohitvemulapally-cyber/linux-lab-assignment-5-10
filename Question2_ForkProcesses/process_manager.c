/*
 * process_manager.c
 *
 * Simulates a "web server" style parent process that:
 *   1. Creates multiple child processes using fork()
 *   2. Monitors their execution
 *   3. Prevents zombie processes (parent reaps children with wait/waitpid)
 *   4. Terminates unresponsive children using signals (SIGTERM, then SIGKILL)
 *
 * Compile: gcc process_manager.c -o process_manager
 * Run:     ./process_manager
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define NUM_CHILDREN 4

int main() {
    pid_t pids[NUM_CHILDREN];

    printf("Parent PID: %d\n", getpid());
    printf("Creating %d child processes...\n\n", NUM_CHILDREN);

    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            // fork failed
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            // ---- CHILD PROCESS ----
            srand(getpid());
            int work_time = (rand() % 5) + 1;   // simulate 1-5 seconds of "work"

            // Child 2 (index 2) will deliberately "hang" to simulate an
            // unresponsive request handler, so we can demonstrate killing it.
            if (i == 2) {
                printf("[Child %d] PID %d is UNRESPONSIVE (simulating a hang)...\n", i, getpid());
                while (1) {
                    sleep(1); // hangs forever until killed by parent
                }
            }

            printf("[Child %d] PID %d starting work, will take %d seconds\n", i, getpid(), work_time);
            sleep(work_time);
            printf("[Child %d] PID %d finished work, exiting normally\n", i, getpid());
            exit(EXIT_SUCCESS);
        }
        else {
            // ---- PARENT PROCESS ----
            pids[i] = pid;
            printf("[Parent] Created child %d with PID %d\n", i, pid);
        }
    }

    // ---- Parent monitors children ----
    printf("\n[Parent] Monitoring children...\n");

    int finished = 0;
    time_t start_time = time(NULL);
    const int TIMEOUT = 4; // seconds parent will wait before declaring a child unresponsive

    while (finished < NUM_CHILDREN) {
        for (int i = 0; i < NUM_CHILDREN; i++) {
            if (pids[i] == -1) continue; // already handled

            int status;
            pid_t result = waitpid(pids[i], &status, WNOHANG);

            if (result == 0) {
                // Child still running -- check if it has exceeded the timeout
                if (time(NULL) - start_time > TIMEOUT) {
                    printf("[Parent] Child %d (PID %d) exceeded timeout. Sending SIGTERM...\n", i, pids[i]);
                    kill(pids[i], SIGTERM);
                    sleep(1);

                    // Check again -- if still alive, force kill
                    result = waitpid(pids[i], &status, WNOHANG);
                    if (result == 0) {
                        printf("[Parent] Child %d (PID %d) still alive. Sending SIGKILL...\n", i, pids[i]);
                        kill(pids[i], SIGKILL);
                        waitpid(pids[i], &status, 0); // blocking wait to reap it -- prevents zombie
                    }
                    printf("[Parent] Child %d (PID %d) terminated and reaped.\n", i, pids[i]);
                    pids[i] = -1;
                    finished++;
                }
            }
            else if (result == pids[i]) {
                // Child exited naturally -- reap it immediately (prevents zombie)
                if (WIFEXITED(status)) {
                    printf("[Parent] Child %d (PID %d) exited normally with status %d. Reaped.\n",
                           i, pids[i], WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("[Parent] Child %d (PID %d) was killed by signal %d. Reaped.\n",
                           i, pids[i], WTERMSIG(status));
                }
                pids[i] = -1;
                finished++;
            }
        }
        usleep(200000); // poll every 0.2s
    }

    printf("\n[Parent] All children handled. No zombies remain. Exiting.\n");
    return 0;
}
