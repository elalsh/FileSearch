# Bug 1

## A) How is your program acting differently than you expect it to?
- ISPathSafe unit tests were failing, files that are not contained withing
  the root were being returned as true. I got the absolute path
  and was comparing the first root absolute path length characters.

## B) Brainstorm a few possible causes of the bug
- Index error, like when passing the lenght to strcmp
- Specifically the if statement used to check and return false are
  not functionning properly
-

## C) How you fixed the bug and why the fix was necessary
- While looking at the unit test it was failing, I sat even if the
  first root absolute path length characters were the same, I had to
  include test_abs_path[root_abs_path_len] != '/' to make sure, we returned
  false otherwise and I was able to pass the test


# Bug 2

## A) How is your program acting differently than you expect it to?
- I was getting memory leak when running the HttpUtils.c under valgrind

## B) Brainstorm a few possible causes of the bug
- I didn't recieve a seg fault so it must be unfreed blocks
- Not freing in all the possible cases likely the issue
-

## C) How you fixed the bug and why the fix was necessary
- When getting the absolute path of root and file in http util,
  I passed in nullptr for 2nd arg, so I had to free the result I stored.
  If the returned val was null, I returned false, so when I used real path
  for the file, I returned false without freind the allocated char* for
  root absolute path.
  char* test_abs_path = realpath(test_file.c_str(), nullptr);
  if (test_abs_path == nullptr) {
    free(root_abs_path);
    return false;
  }


# Bug 3

## A) How is your program acting differently than you expect it to?
- The result of the search didn't correctly match the solution binary when
  running the server and searching for queries, the part where it says
  how many results where found

## B) Brainstorm a few possible causes of the bug
- Didn't properly parse the query correctly
-
-

## C) How you fixed the bug and why the fix was necessary
- I was able to figure out the problem when I realized we have to
  pass the queries as lower case, so before passing the list of strings,
  I used to_lower(string) to pass the argument to query processor correcly
