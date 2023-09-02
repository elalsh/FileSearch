# Bug 1

## A) How is your program acting differently than you expect it to?
- I was getting a seg fault when I run the fileparser tests and It
  took me a while to figure out the bug.

## B) Brainstorm a few possible causes of the bug
- Referencing null address
- Freeing or passing & of a pointer
-

## C) How you fixed the bug and why the fix was necessary
- I wasn't passing words correctly to create the hashtable


# Bug 2

## A) How is your program acting differently than you expect it to?
- Again I was getting a Seg fault when working on CrawlFileTree

## B) Brainstorm a few possible causes of the bug
- Referencing null address
-  Freeing or passing & of a pointer
-

## C) How you fixed the bug and why the fix was necessary
- I was passing the file_path instead of the file content
  to ParseWordsIntoHashtable


# Bug 3

## A) How is your program acting differently than you expect it to?
- Getting a memory leak when running valgrind on memindex

## B) Brainstorm a few possible causes of the bug
- Not freing all the spaces allocated
- Double free
-

## C) How you fixed the bug and why the fix was necessary
- Freing the Iterator after using it
