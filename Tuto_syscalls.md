# Syscalls en x86 (x86)
---

## ⚙️ Pasos para crearla

### 1. Agrega el handler (en `kernel/sys.c`)

```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(mi_syscall, int, x) {
    printk(KERN_INFO "Syscall activada con valor: %d\n", x);
    return x * 2;
}

SYSCALL_DEFINE2(suma_valores, int, a, int, b) {
    int resultado = a + b;
    printk(KERN_INFO "[Syscall suma_valores] a = %d, b = %d, resultado = %d\n", a, b, resultado);
    return resultado;
}
```
¿Qué pasa si el codigo usa otras syscalls? 
Es mejor no hacerlo, pues se puedan causar importaciones circulares
---

### 2. Asigna número en `arch/x86/entry/syscalls/syscall_64.tbl`

Justo en la ultima linea vamos a encontrar
```
548    common    mi_syscall       __x64_sys_mi_syscall
549    common    suma_valores     __x64_sys_suma_valores
```

---

### 3. Declara en `include/linux/syscalls.h`

```c
asmlinkage long sys_mi_syscall(int x);
asmlinkage long sys_suma_valores(int a, int b);
```

---

### 4. Recompila e instala

```bash
make -j$(nproc)
sudo make modules_install
sudo make install
sudo reboot
```

---

## Código en C para probarla

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define __NR_mi_syscall 548
#define __NR_suma_valores 549

int main() {
    long r1 = syscall(__NR_mi_syscall, 21);
    long r2 = syscall(__NR_suma_valores, 5, 7);
    printf("Resultado mi_syscall: %ld\n", r1);
    printf("Resultado suma_valores: %ld\n", r2);
    return 0;
}
```

```bash
gcc test.c -o test
./test
dmesg | tail
```

---

## 🧩 Detalles técnicos

| Registro | Uso                |
| -------- | ------------------ |
| `rax`    | número de syscall  |
| `rdi`    | arg1 (ej. `int x`) |
| `rsi-r9` | arg2–6             |
| `rax`    | valor de retorno   |

---

## ✅ Tips

* Verifica que los números `548` y `549` no estén ocupados.
* Usa `printk()` para debug.
* Verifica con `dmesg`.
