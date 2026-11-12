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
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

struct test_rwlock_args {
    pthread_rwlock_t simple_lock;
    int simple_counter;
    int target_writes;
};

static void test_rwlock(test_rwlock_args *args) {
    int n_writes = 0;
    while (n_writes < args->target_writes) {
        if (rand() & 1) {
            pthread_rwlock_wrlock(&args->simple_lock);
            args->simple_counter += 1;
            pthread_rwlock_unlock(&args->simple_lock);
            n_writes += 1;
        } else {
            pthread_rwlock_rdlock(&args->simple_lock);
            if (args->simple_counter & 1) {
                sched_yield();
            }
            pthread_rwlock_unlock(&args->simple_lock);
        }
    }
    printf("%u: %s done\n", gettid(), __FUNCTION__);
}

static void *test_rwlock_thread(void *arg) {
    test_rwlock(static_cast<test_rwlock_args *>(arg));
    return arg;
}

int test_main (int argc __attribute__((__unused__)), const char *argv[] __attribute__((__unused__))) {
    test_rwlock_args args = { PTHREAD_RWLOCK_INITIALIZER, 0, 1000000 };
    pthread_t pid;
    pthread_create(&pid, nullptr, test_rwlock_thread, &args);
    test_rwlock(&args);
    void *ret;
    pthread_join(pid, &ret);
    printf("%s: simple_counter = %d\n", __FUNCTION__, args.simple_counter);
    return 0;
}
