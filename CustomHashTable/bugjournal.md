# Bug 1

## A) How is your program acting differently than you expect it to?
-  For a Push and Pop linkedlist, I want failing the unit test and was not passsing
   all tests, it's say equality fails

## B) Brainstorm a few possible causes of the bug
- Not updating num elements
- updating the elements repeatedly whe using already defined function
-

## C) How you fixed the bug and why the fix was necessary
- I checked the console output to find where (line number in the test) it was
  failing and opened the tests to see what is causing it to fail. The num_elem
  emnts was not correct so I realized it must be because of not updating
  the num_elements correctly and fixed it. Decrement when removeing, increment
  when adding.


# Bug 2

## A) How is your program acting differently than you expect it to?
- I was getting a seg fault when I tried to get the payload and store it
  keyvalue for find, remove, insert in hashtable. I was passing the address
  of keyvalue output parameter, which resulted in seg fault

## B) Brainstorm a few possible causes of the bug
- Trying to access invalid memeory
- referencing invalid memory(NULL)
- Being carefull when passing pointers (taking their address and dereferencing)

## C) How you fixed the bug and why the fix was necessary
- since we are using the iterator on specific bucket, I iterated over elements
  in the bucket to find the elements with the same key. Instead of taking the
  address of output parameter keyvalue, I created a local HTKeyValue_t* and
  passed that to get and remove method of LLIterator to get the payload. From
  then on, I didn't get seg fault error.

# Bug 3

## A) How is your program acting differently than you expect it to?
- I was getting a memory leak when I run valgrin on the full test_suite.

## B) Brainstorm a few possible causes of the bug
- Since the valgrind for Linked List passed, the error must be in the Hash
  table and I must have values that not being freed.
- Insert is where we mallocced for HTKeyValue_t* to add to the bucker elements
  so it could be in insert method
- Make sure to check helper function to find things we might have to free.

## C) How you fixed the bug and why the fix was necessary
- In the helper function I used to iterate the elements of the bucker,
  I didn't free the iterator I used and the local variable  HTKeyValue_t* I
  created to pass into the get and remove function. I freed the iterator and
  and also called free on the  HTKeyValue_t* if we wanted to delete the
  element from the bucker
