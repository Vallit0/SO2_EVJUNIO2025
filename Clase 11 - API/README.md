# API en C con Ulfius y Jansson

## Instalar Ulfius
Ulfius está disponible en múltiples distribuciones como paquete oficial.

```bash
$ sudo apt install libulfius-dev uwsc
```

## Instalar Jansson
La instalación de Jansson deberá de realizarse de forma manual.

El codigo fuente de Jansson está disponible en http://www.digip.org/jansson/releases/.

Desempaquete el archivo tar de origen y cambie al directorio de origen:

```bash
$ #Reemplazar la X.Y.Z con el número de version descargada
$ bunzip2 -c jansson-X.Y.Z.tar.bz2 | tar xf -
$ cd jansson-X.Y.Z
```

Luego se creará una carpeta ```build``` y se generarán los archivos ```make``` utilizando ```CMake```

```bash
$ mkdir build
$ cd build
$ cmake .. # o ccmake .. para utilizar una GUI.
```
Una vez creados los archivos de ```make``` se compilará
```bash
$ make
$ make check
$ make install
```

## Compilar API
Una vez hecha la API con Ulfius + Jansson, se compilará con GCC, agregando las flags para incluir las librerias.

```bash
$ gcc api.c -o api -lulfius -ljansson
```