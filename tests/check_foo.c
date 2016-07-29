/*
 * nonet - execute a command without network access
 * Copyright (C) 2016 Israel G. Lugo
 *
 * This file is part of nonet.
 *
 * nonet is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * nonet is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with nonet. If not, see <http://www.gnu.org/licenses/>.
 *
 * For suggestions, feedback or bug reports: israel.lugo@lugosys.com
 */


#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <check.h>

START_TEST(test_one)
{
    /* stuff */
    int i = 5;
    int j = 0;

    ck_abort_msg("Die, die, die!");
}
END_TEST


Suite *foo_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("Foo");

    tc = tcase_create("Core");

    tcase_add_test(tc, test_one);
    suite_add_tcase(s, tc);

    return s;
}


int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = foo_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
