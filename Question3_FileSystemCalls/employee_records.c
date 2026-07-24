/*
 * employee_records.c
 *
 * Secure file-processing utility using Linux SYSTEM CALLS
 * (open, read, write, lseek, close) instead of standard library
 * functions like fopen/fprintf.
 *
 * Demonstrates:
 *   1. Creating a file
 *   2. Writing fixed-length employee records
 *   3. Updating a specific record in place (no full rewrite) using lseek()
 *   4. Retrieving a record from any position efficiently (random access)
 *
 * Compile: gcc employee_records.c -o employee_records
 * Run:     ./employee_records
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define RECORD_SIZE 40   // fixed size per record (in bytes)
#define ID_LEN 4
#define NAME_LEN 20
#define SALARY_LEN 10
#define FILE_NAME "employees.dat"

// A fixed-length record structure: ID(4) + NAME(20) + SALARY(10) padded to 40 bytes
typedef struct {
    char id[ID_LEN];
    char name[NAME_LEN];
    char salary[SALARY_LEN];
    char padding[RECORD_SIZE - ID_LEN - NAME_LEN - SALARY_LEN];
} Employee;

void write_record(int fd, int record_num, const char *id, const char *name, const char *salary) {
    Employee emp;
    memset(&emp, 0, sizeof(Employee));
    strncpy(emp.id, id, ID_LEN);
    strncpy(emp.name, name, NAME_LEN);
    strncpy(emp.salary, salary, SALARY_LEN);

    // lseek: move file offset to the exact byte position of this record
    off_t offset = lseek(fd, record_num * RECORD_SIZE, SEEK_SET);
    if (offset == -1) {
        perror("lseek failed");
        exit(EXIT_FAILURE);
    }

    ssize_t written = write(fd, &emp, sizeof(Employee));
    if (written != sizeof(Employee)) {
        perror("write failed");
        exit(EXIT_FAILURE);
    }
}

void read_record(int fd, int record_num) {
    Employee emp;
    memset(&emp, 0, sizeof(Employee));

    // lseek directly to the record's byte offset -- O(1) random access,
    // no need to read through preceding records.
    off_t offset = lseek(fd, record_num * RECORD_SIZE, SEEK_SET);
    if (offset == -1) {
        perror("lseek failed");
        return;
    }

    ssize_t bytes_read = read(fd, &emp, sizeof(Employee));
    if (bytes_read <= 0) {
        printf("No record found at position %d\n", record_num);
        return;
    }

    // Copy into null-terminated local buffers before printing (the stored
    // fields are fixed-width and not guaranteed to be null-terminated).
    char id_str[ID_LEN + 1] = {0};
    char name_str[NAME_LEN + 1] = {0};
    char salary_str[SALARY_LEN + 1] = {0};
    memcpy(id_str, emp.id, ID_LEN);
    memcpy(name_str, emp.name, NAME_LEN);
    memcpy(salary_str, emp.salary, SALARY_LEN);

    printf("Record %d -> ID: %-4s | Name: %-20s | Salary: %-10s\n",
           record_num, id_str, name_str, salary_str);
}

int main() {
    // 1. Create/open the file for read+write, create if not exists, truncate if exists
    int fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }
    printf("Step 1: File '%s' created/opened (fd=%d)\n\n", FILE_NAME, fd);

    // 2. Write employee records
    printf("Step 2: Writing 4 employee records...\n");
    write_record(fd, 0, "E001", "Alice Johnson", "55000");
    write_record(fd, 1, "E002", "Bob Smith", "48000");
    write_record(fd, 2, "E003", "Carol Lee", "62000");
    write_record(fd, 3, "E004", "David Kim", "51000");
    printf("Done.\n\n");

    // 3. Retrieve records from arbitrary positions (random access)
    printf("Step 3: Reading records in non-sequential order (2, 0, 3, 1)...\n");
    read_record(fd, 2);
    read_record(fd, 0);
    read_record(fd, 3);
    read_record(fd, 1);
    printf("\n");

    // 4. Update a specific record (Bob Smith's salary) WITHOUT rewriting the whole file
    printf("Step 4: Updating record 1 (Bob Smith) salary only...\n");
    write_record(fd, 1, "E002", "Bob Smith", "60000");
    printf("Done.\n\n");

    // 5. Verify the update
    printf("Step 5: Verifying updated record 1 and confirming others are unchanged...\n");
    read_record(fd, 0);
    read_record(fd, 1);
    read_record(fd, 2);
    read_record(fd, 3);

    close(fd);
    printf("\nFile closed. All operations completed using raw syscalls only.\n");

    return 0;
}
