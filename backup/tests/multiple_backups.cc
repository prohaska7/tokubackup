/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
/*======
This file is part of Percona TokuBackup.

Copyright (c) 2006, 2015, Percona and/or its affiliates. All rights reserved.

    Percona TokuBackup is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2,
    as published by the Free Software Foundation.

     Percona TokuBackup is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Percona TokuBackup.  If not, see <http://www.gnu.org/licenses/>.

----------------------------------------

    Percona TokuBackup is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License, version 3,
    as published by the Free Software Foundation.

    Percona TokuBackup is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with Percona TokuBackup.  If not, see <http://www.gnu.org/licenses/>.
======= */

#ident "$Id$"

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "backup.h"
#include "backup_test_helpers.h"

const int TRIES = 16;
char *dirs[TRIES];

static void setup(void)
{
    for (int i = 0; i < TRIES; ++i) {
        const int LEN = 200;
        char s[LEN];
        int r = snprintf(s, sizeof(s), "%s.backup_%d", get_test_name(), i);
        check(r < LEN);
        systemf("rm -rf %s", s);
        dirs[i] = strdup(s);
        check(dirs[i]);
    }
}

static void breakdown(void) {
    // Note: since dir names freed here, they
    // must be strdup'ed when starting the backup
    for (int i = 0; i < TRIES; ++i) {
        free(dirs[i]);
    }
}

static void write_lots_of_dummy_data(char *src)
{
    const int SIZE = 100;
    const int MB = 1024 * 1024;
    char dummy[MB] = {0};
    char file[SIZE];
    snprintf(file, SIZE, "%s/%s", src, "dummy");
    int fd = open(file, O_CREAT | O_RDWR, 0777);
    check(fd >= 0);
    const int MEGS = 10;
    for (int i = 0; i < MEGS; ++i) {
        int r = write(fd, dummy, MB);
        check(r == MB);
    }
}

static void work_it(char *src, char *magic, int size)
{
    const int FILE_MAX = 200;
    char file[FILE_MAX];
    snprintf(file, FILE_MAX, "%s/%s", src, magic);

    // 1.  Create a file.
    int fd = open(file, O_CREAT | O_RDWR, 0777);
    check(fd >= 0);
    // 2.  Write magic to the file.
    int r = write(fd, magic, size);
    check(r == size);

    // 3.  Close the file.
    close(fd);

    // 4.  Open the file.
    fd = open(file, O_RDWR);

    // 5.  Read from the file.
    char *buf = (char*)malloc(sizeof(char) * size);
    r = read(fd, buf, size);

    // 6.  Write magic to the file.
    lseek(fd, size, SEEK_SET);
    r = write(fd, magic, size);

    // 7.  pwrite magic to the file.
    r = pwrite(fd, magic, size, size * 3);

    // 8.  ftruncate magic from the file.
    r = ftruncate(fd, size * 3);

    // close the file.
    close(fd);
    free((void*)buf);
}


static int multiple_backups(void) {
    setup_source();
    setup();
    char * src = get_src();
    write_lots_of_dummy_data(src);

    int result = 0;

    pthread_t thread;
    tokubackup_throttle_backup(1L << 22);
    for (int i = 0; i < TRIES; ++i) {
        setup_directory(dirs[i]);
        start_backup_thread(&thread, strdup(dirs[i]));

        const int SIZE = 9;
        char buf[SIZE] = {0};
        int r;
        r = snprintf(buf, SIZE, "magic%d", i);
        check(r < SIZE);
        work_it(src, buf, SIZE);

        finish_backup_thread(thread);

        // diff the source and backup directories
        r = systemf("diff -r %s %s", src, dirs[i]);
        if (r != 0) {
            printf("%s: diff -r %s %s %d\n", __FUNCTION__, src, dirs[i], r);
            result = -1;
        }
    }

    free(src);
    if(result < 0) {
        printf("Backup # %d failed.\n", result);
        fail();
        result = -1;
    } else {
        pass();
    }

    breakdown();

    return result;
}

int test_main(int argc __attribute__((__unused__)), const char *argv[] __attribute__((__unused__))) {
    return multiple_backups();
}
