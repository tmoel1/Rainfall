## Level 0

### Vulnerability Analysis
The target binary is `level0`, which is extremely basic. 
Running the binary natively returns a `Segmentation fault (core dumped)` with no arguments, and returns `No !` with a dummy argument (like `123`).

To understand the core logic of the program, we extract the assembly of the `main` function using `gdb`. The commands used are:
* `gdb -q ./level0`: launches the GNU debugger quietly (without the startup banner).
* `set disassembly-flavor intel`: changes the syntax from AT&T to the more readable Intel assembly syntax.
* `disas main`: dumps the assembly Code for the main function.

By disassembling the `main` function, we can observe the following critical logic:
1. It grabs `argv[1]` (`mov eax,DWORD PTR [ebp+0xc]; add eax,0x4`).
2. It passes `argv[1]` to `atoi` (`call 0x8049710 <atoi>`). This indicates the program expects an integer as its first argument.
3. It compares the returned integer against `0x1a7` (`cmp eax,0x1a7`).
4. Hexadecimal `0x1a7` converts to `423` in decimal.
5. If the comparison doesn't match, it jumps to printing "No !" and exiting.
6. If it matches, it calls `geteuid`, `getegid`, `setresuid`, and `setresgid` to grant us the effective privileges of the file's owner (`level1`), and then it executes a bash shell using `execv`.

### The Exploit
Knowing that the program expects the specific integer `423` as its first argument, we can exploit this to gain the elevated shell and read the password for `level1`.

```bash
level0@RainFall:~$ ./level0 423
$ cat /home/user/level1/.pass
1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a
```
