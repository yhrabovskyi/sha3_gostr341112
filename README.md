This is a part of work for the article. Here we have implementation of SHA-3 based on Keccak specifications, version 2
and GOST R 34.11-2012. Main purpose is to measure:
  1. How many clocks algorithm is needed for process one byte of a long message;
  2. How much memory is needed for the algorithm;
  3. Size of code.

There is only the source code without full projects. Measurement was carried out as iteration, which represents 102396
bytes of the message. Also it was carried out with using IAR EW simulator. Comments have been written in Ukrainian,
so there can be some problems with encoding. 2 implementations have been written in C. The length of message digests is 512 bits.

October, 2013.