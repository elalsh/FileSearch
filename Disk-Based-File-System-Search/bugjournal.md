# Bug 1
 When implementing the write hashtable, I was trying to implement
 the write bucket and the bucket record in the same function
## A) How is your program acting differently than you expect it to?
- I wasn't passing the write index test and was getting error for the
  100000 < res test.

## B) Brainstorm a few possible causes of the bug
- Seeking to the wrong place
- writing the wrong data
-

## C) How you fixed the bug and why the fix was necessary
- used the writebucket and record function in the write hash table function.
  This way I got back the number of bytes written so it was easier to keep
  track of where to seek to and cheking for error.


# Bug 2

## A) How is your program acting differently than you expect it to?
- Checksum was failing and retuning the wrong number and I was failling the
  test for write index.

## B) Brainstorm a few possible causes of the bug
- Possibly, the data written is wrong
- Not using the crc object correctly
- seeking to the wrong place

## C) How you fixed the bug and why the fix was necessary
- I was reading the data, byte by byte starting from 0 to hi+in bytes,
  I seeked to sizeof(indexfileheader) to pass to the crc object and
  passed the test.


# Bug 3

## A) How is your program acting differently than you expect it to?
- I was failing fwrite at step 18, when trying to write the string and thus
  I was returning, KFailedWrite

## B) Brainstorm a few possible causes of the bug
- possibly seeking to the wrong spot
- reading the wrong number of bytes
-

## C) How you fixed the bug and why the fix was necessary
- I was using the same fwrite() != 1, return KFailedWrite condition for error.
  After reading the man fwrite, I i figured out,
  the return of fwrite for this case should be word_bytes. It worked after
  I fixed that.
