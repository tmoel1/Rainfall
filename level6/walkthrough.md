## Level 6

### Vulnerability Analysis
The target is `level6`. This level introduces Heap-based buffer overflows.

Analyzing the `main` function in `gdb` shows the following logic:
1. `malloc(0x40)` (64 bytes) is called, allocating space on the heap, and its pointer is stored at `esp+0x1c` (`dest`).
2. `malloc(0x4)` (4 bytes) is called, allocating a second block on the heap, and its pointer is stored at `esp+0x18` (`func`).
3. The address of the function `m` (`0x08048468`) is written into the memory pointed to by `func`.
4. `strcpy(dest, argv[1])` is heavily vulnerable. It copies the first command-line argument into the 64-byte `dest` buffer. `strcpy` doesn't check lengths, so we can overflow `dest`. 
5. Because memory chunks are placed contiguously on the heap, overflowing `dest` allows us to overwrite the contents of `func`.
6. Finally, the program calls the function pointer stored in `func`: `call eax`.

There is another function `n` located at `0x08048454` which calls `system("/bin/cat /home/user/level7/.pass")` (or similar). 

### The Exploit
Since the first allocation is 64 bytes (`0x40`), and `malloc` uses an 8-byte chunk header for metadata on 32-bit systems, the distance between the user data of the first chunk and the user data of the next adjacent chunk is exactly `64 + 8 = 72` bytes.

Therefore, our `strcpy` payload simply needs to provide 72 bytes of padding, followed by the memory address of `n` (`0x08048454`). Because `argv[1]` is placed in `dest`, it will correctly overflow and inject the new pointer directly into `func`.

Payload: `[72 bytes of padding] + [Address of n()]`

```bash
level6@RainFall:~$ ./level6 $(python -c 'print "A" * 72 + "\x54\x84\x04\x08"')
f73dcb7a06f60e3ccc608990b0a046359d42a1a0489ffeefd0d9cb2d7c9cb82d
```