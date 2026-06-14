## Level 8

### Vulnerability Analysis
The target is `level8`. This level is purely a reverse-engineering and logic challenge. Unlike previous levels, the exploit involves manipulating the layout of chunks on the heap through specific program inputs rather than injecting explicit padding or shellcode.

Analyzing `main` in GDB, we observe an infinite loop that takes standard user commands:
1. It prints two static memory pointers (`auth` and `service`) at the start of every loop mapping to `0x8049aac` and `0x8049ab0`.
2. `auth [text]`: If the user inputs `auth`, `malloc(4)` is called. The pointer is stored in the `auth` global variable. A 0 is written to `auth[0]`. Then whatever was typed after `auth ` is copied directly into `auth` using `strcpy()`, as long as it is 30 characters or less.
3. `reset`: This explicitly frees the `auth` pointer.
4. `service [text]`: This uses `strdup` (which calls `malloc`) to copy the text after `service` to the heap, storing the pointer in the `service` global variable.
5. `login`: This checks if the 32nd byte of `auth` (auth + 32) is not zero. If it is non-zero, it invokes `system("/bin/sh")`. Otherwise, it prints `Password:\n`.

### The Exploit
To bypass the login check, we simply need whatever is sitting 32 bytes past the start of the `auth` structure to be non-zero. 

When `auth ` allocates memory, it reserves 4 bytes, but the system allocates a 16-byte chunk size.
When `service ` allocates memory, it creates the very next chunk on the heap adjacent to the `auth` chunk.
The start of the `service` chunk will fall exactly at `auth + 16` bytes. Therefore, any data inside the `service` chunk easily spans across the `auth + 32` boundary being checked by `login`.

By allocating `auth ` and then immediately allocating a long `service ` string, we populate the heap exactly where `login` checks. 

```bash
level8@RainFall:~$ ./level8 
(nil), (nil) 
auth 
0x804a008, (nil) 
service AAAAAAAAAAAAAAAAAAAAAAAAA
0x804a008, 0x804a018 
login
$ cat /home/user/level9/.pass
c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
```