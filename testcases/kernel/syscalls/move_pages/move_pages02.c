/*
 *   Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 *
 *   Based on testcases/kernel/syscalls/waitpid/waitpid01.c
 *   Original copyright message:
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	move_pages02.c
 *
 * DESCRIPTION
 *      Test movement of pages mapped by a process.
 *
 * ALGORITHM
 *      1. Allocate pages in NUMA node A.
 *      2. Use move_pages() to move the pages to NUMA node B.
 *      3. Retrieve the NUMA nodes of the moved pages.
 *      4. Check if all pages are in node B.
 *
 * USAGE:  <for command-line>
 *      move_pages02 [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	05/2008 Vijay Kumar
 *		Initial Version.
 *
 * Restrictions
 *	None
 */

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <numa.h>

#include <test.h>
#include <usctest.h>

#include "move_pages_support.h"

#define TEST_PAGES 2
#define TEST_NODES 2

void setup(void);
void cleanup(void);

char *TCID = "move_pages02";
int TST_TOTAL = 1;
extern int Tst_count;

int main(int argc, char **argv)
{
	unsigned int i;
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	unsigned int from_node = 0;
	unsigned int to_node = 1;

	/* parse standard options */
	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
		/* NOTREACHED */
	}

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		void *pages[TEST_PAGES] = { 0 };
		int nodes[TEST_PAGES];
		int status[TEST_PAGES];
		int ret;

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		ret = alloc_pages_on_node(pages, TEST_PAGES, from_node);
		if (ret == -1)
			continue;

		for (i = 0; i < TEST_PAGES; i++)
			nodes[i] = to_node;

		ret = numa_move_pages(0, TEST_PAGES, pages, nodes, status, MPOL_MF_MOVE);
		TEST_ERRNO = errno;
		if (ret != 0) {
			tst_resm(TFAIL, "retrieving NUMA nodes failed");
			free_pages(pages, TEST_PAGES);
			continue;
		}

		for (i = 0; i < TEST_PAGES; i++)
			*((char *)pages[i]) = 0xAA;

		verify_pages_on_node(pages, status, TEST_PAGES, to_node);

		free_pages(pages, TEST_PAGES);
	}

	cleanup();
	/* NOT REACHED */

	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void
setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	check_config(TEST_NODES);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion
 */
void
cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
	/*NOTREACHED*/
}