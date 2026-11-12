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

static int simple_counter = 0;

static void *simple_writer_thread(void *arg) {
    simple_counter += 1;
    return arg;
}

static void test_simple_data_race(void) {
    simple_counter = 0;
    pthread_t pid;
    pthread_create(&pid, nullptr, simple_writer_thread, nullptr);
    simple_counter += 1;
    void *ret;
    pthread_join(pid, &ret);
    printf("%s: simple_counter = %d\n", __FUNCTION__, simple_counter);
}

int test_main (int argc __attribute__((__unused__)), const char *argv[] __attribute__((__unused__))) {
    test_simple_data_race();
    return 0;
}
