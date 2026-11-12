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

#include <stdio.h>
#include <pthread.h>

static pthread_rwlock_t simple_lock = PTHREAD_RWLOCK_INITIALIZER;
static int simple_counter = 0;

static void do_write_with_rdlock(void) {
    pthread_rwlock_rdlock(&simple_lock);
    simple_counter += 1;
    pthread_rwlock_unlock(&simple_lock);
}

static void *writer_thread(void *arg) {
    do_write_with_rdlock();
    return arg;
}

int test_main (int argc __attribute__((__unused__)), const char *argv[] __attribute__((__unused__))) {
    simple_counter = 0;
    pthread_t pid;
    pthread_create(&pid, nullptr, writer_thread, nullptr);
    do_write_with_rdlock();
    void *ret;
    pthread_join(pid, &ret);
    printf("%s: simple_counter = %d\n", __FUNCTION__, simple_counter);
    return 0;
}
