/*
 * Copyright ©2023 Chris Thachuk.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "libhw1/CSE333.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc
static void Usage(void);

// helper function to make letter of the str lowercase
static void smallLetter(char* str, int len);

//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char** argv) {
  if (argc != 2) {
    Usage();
  }

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - Crawl from a directory provided by argv[1] to produce and index
  //  - Prompt the user for a query and read the query from stdin, in a loop
  //  - Split a query into words (check out strtok_r)
  //  - Process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.
  //
  // Note that you should make sure the fomatting of your
  // searchshell output exactly matches our solution binaries
  // to get full points on this part.
  DocTable* dt;
  MemIndex* mi;

  dt = DocTable_Allocate();
  mi = MemIndex_Allocate();

  printf("Indexing \'%s\'\n", argv[1]);

  if(!CrawlFileTree(argv[1], &dt, &mi)) {
    Usage();
  }
  char** list = (char**) malloc(100*sizeof(char*));

  while (1) {
    char buff[1024];
    printf("enter query:\n");
    if (fgets(buff, sizeof(buff), stdin) == NULL) {
      fprintf(stderr, "shutting down...\n");
      break;
    }
    buff[strlen(buff)-1] = '\0';
    char* savePtr;

    int index = 0;
    char* token = strtok_r(buff, " ", &savePtr);
    while (token != NULL) {
      list[index] = (char*) malloc((strlen(token) + 1));
      list[index] = strcpy(list[index], token);
      smallLetter(list[index], strlen(token) + 1);
      index++;
      token = strtok_r(NULL, " ", &savePtr);
    }

    LinkedList* lsr = MemIndex_Search(mi, list, index);
    if (lsr != NULL) {
      LLIterator* ll = LLIterator_Allocate(lsr);
      SearchResult *rs;

      while (LLIterator_IsValid(ll)) {
        LLIterator_Get(ll, (LLPayload_t*)&rs);
         printf("  %s (%u)\n", DocTable_GetDocName(dt, rs->doc_id), rs->rank);
         LLIterator_Next(ll);
      }
      LLIterator_Free(ll);
    }

  }
  free(list);
  DocTable_Free(dt);
  MemIndex_Free(mi);
  return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}

static void smallLetter(char* str, int len) {
  for (int i = 0; i < len; i++) {
    *(str + i) = tolower(*(str+i));
  }
}

