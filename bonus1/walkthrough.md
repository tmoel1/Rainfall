## Bonus 1

### Vulnerability Analysis
The target is `bonus1`. 
Disassembling `main` in GDB, we observe:
1. It takes two command line arguments: `argv[1]` and `argv[2]`.
2. It converts `argv[1]` to an integer using `atoi(argv[1])`, and stores it in a local variable `size` at `esp+0x3c`.
3. It checks if `size` is greater than `9`. If it is, the program immediately returns `1` and exits.
4. It then copies the second argument (`argv[2]`) into a `40` byte local buffer (`esp+0x14`) using `memcpy(buffer, argv[2], size * 4)`.
5. Finally, it checks if `size` is strictly equal to `0x574f4c46` (which is `1464814662` in decimal). If so, it pops a shell using `execl("/bin/sh")`.

The vulnerability originates from the condition `size <= 9` combined with `size * 4`. The `atoi` function returns a standard signed 32-bit integer. If we provide a **negative number**, it bypasses the `size > 9` check (e.g. `-10` is less than `9`). 

But how does it pass the `size == 1464814662` check? It doesn't! We can't reach the `execl` call because `size` would need to simultaneously be negative AND strictly equal to a huge positive number.

However, we can just execute a classic Buffer Overflow on the stack! `memcpy`'s third parameter is a `size_t`, which expects an **unsigned** integer. When the integer `-x` is converted to unsigned, it wraps around to a massive positive number (e.g. `4,294...`). This means `memcpy` will copy the entirety of `argv[2]` without stopping at 40 bytes!

### The Exploit
While we could inject Shellcode, there is an easier method natively baked into the logic.
```assembly
0x08048478 <+84>:    cmp    DWORD PTR [esp+0x3c],0x574f4c46
...
0x08048499 <+117>:   call   0x8048350 <execl@plt>
```
If the `size` variable equals `0x574f4c46` (which represents the ASCII string "FLOW", or `1464814662` in decimal), the program automatically executes a shell!

The `size` variable is stored on the stack right below `buffer` (at offset `esp+0x3c`). The `buffer` itself is stored at `esp+0x14`.
The distance from `buffer` to `size` is exactly `0x3c - 0x14` = **40 bytes**.

We override the copy-length limit by providing `argv[1]` as a negative number. However, because `memcpy` takes `size * 4`, supplying `-11` behaves unexpectedly. `-11` (`0xfffffff5`) multiplied by 4 overflows into `0xffffffd4` (1073741780 unsigned), which works. Wait, a segmentation fault implies `memcpy` read far past our 40-byte string until it hit inaccessible memory, because it tried to copy 1 billion bytes.

Instead, we simply provide an exact length integer! If we pass `-1073741813`, the multiplication `x * 4` forces it to overflow down exactly back to `44` bytes!
Wait, that is too complex and the OS may truncate the math wrap depending on alignment.

Actually, using `1073741824` `* 4` = `4294967300`, which wraps exactly around the 32-bit ceiling `0`.
If we provide `1073741835` (e.g., `< 0`), wait, let's trace `argv[1]` mathematically:
Our input size string must equal `44` exactly after multiplying by 4, but be negative before multiplying.
`-x * 4 = 44 (mod 4294967296)`
`-x = 11 (mod 1073741824)` is inaccurate since `-11 * 4` evaluates to `0xffffffd4` (4294967252 bytes) on `memcpy`. The Segfault is because `memcpy` copied all 40 'A's + "FLOW" + garbage up to 4 GB until it struck unmapped memory.

Let's force the `size <= 9` check to fail using `-1073741813`.
`-1073741813 * 4` = `-4294967252` -> Overflowing 32-bit integer constraints = `44` identically.
Because `44` is the unsigned evaluation of the `memcpy` length, it copies exactly 44 bytes and gracefully halts.

`argv[1]` = `-1073741813`
`argv[2]` = `40 "A"s + "FLOW"`

```bash
bonus1@RainFall:~$ ./bonus1 -1073741813 $(python -c 'print "A"*40 + "FLOW"')
$ whoami
bonus2
$ cat /home/user/bonus2/.pass
579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
```