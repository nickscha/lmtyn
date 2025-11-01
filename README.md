# lmtyn
A C89 standard compliant, single header, nostdlib (no C Standard Library) Lucid Modelling Tool You Need (LMTYN).

For more information please look at the "lmtyn.h" file or take a look at the "examples" or "tests" folder.

> [!WARNING]
> THIS PROJECT IS A WORK IN PROGRESS! ANYTHING CAN CHANGE AT ANY MOMENT WITHOUT ANY NOTICE! USE THIS PROJECT AT YOUR OWN RISK!

<p align="center">
  <a href="https://github.com/nickscha/lmtyn/releases">
    <img src="https://img.shields.io/github/v/release/nickscha/lmtyn?style=flat-square&color=blue" alt="Latest Release">
  </a>
  <a href="https://github.com/nickscha/lmtyn/releases">
    <img src="https://img.shields.io/github/downloads/nickscha/lmtyn/total?style=flat-square&color=brightgreen" alt="Downloads">
  </a>
  <a href="https://opensource.org/licenses/MIT">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square" alt="License">
  </a>
  <img src="https://img.shields.io/badge/Standard-C89-orange?style=flat-square" alt="C Standard">
  <img src="https://img.shields.io/badge/nolib-nostdlib-lightgrey?style=flat-square" alt="nostdlib">
</p>

## **Features**
- **C89 compliant** — portable and legacy-friendly  
- **Single-header API** — just include `lmtyn.h`  
- **nostdlib** — no dependency on the C Standard Library  
- **Minimal binary size** — optimized for small executables  
- **Cross-platform** — Windows, Linux, MacOs 
- **Strict compilation** — built with aggressive warnings & safety checks  

## Quick Start

Download or clone lmtyn.h and include it in your project.

```C
#include "lmtyn.h" /* Lucid Modelling Tool You Need */

int main() {
    /* (1) Define a shape (center pos, radius, normal/direction) */
    lmtyn_shape_circle pillar[] = {
      {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f}, /* bottom    */
      {0.0f, 1.0f, 0.0f, 0.6f, 0.0f, 1.0f, 0.0f}, /* low mid   */
      {0.0f, 2.0f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f}, /* center    */
      {0.0f, 3.0f, 0.0f, 0.6f, 0.0f, 1.0f, 0.0f}, /* upper mid */
      {0.0f, 4.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f}, /* top low   */
      {0.0f, 4.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f}  /* top       */
    };

    /* (2) Allocate enough space for vertices and indices buffer */
    lmtyn_mesh mesh = {0};
    mesh.vertices_capacity = sizeof(f32) * 4096;
    mesh.indices_capacity = sizeof(u32) * 4096;
    mesh.vertices = malloc(sizeof(f32) * mesh.vertices_capacity);
    mesh.indices = malloc(sizeof(u32) * mesh.indices_capacity);

     /* (3) Generate the vertices/indices (here we use 4 segments for low-poly look) */
    lmtyn_mesh_generate(
      &mesh,
      0,
      pillar,
      sizeof(pillar) / sizeof(pillar[0]),
      4);

    /* (4) Center Mesh on (0,0,0) and scale to 1 unit */ 
    lmtyn_mesh_normalize(
      &mesh,
      0.0f, 0.0f, 0.0f,
      1.0f);

    /* (5) Now you can use the mesh.vertices / mesh.indices data */

    return 0;
}
```

This is the generated mesh:

<p align="center">
<a href="https://github.com/nickscha/lmtyn"><img src="assets/lmtyn_pillar.gif"></a>
</p>

## Run Example: nostdlib, freestsanding

In this repo you will find the "examples/lmtyn_win32_nostdlib.c" with the corresponding "build.bat" file which
creates an executable only linked to "kernel32" and is not using the C standard library and executes the program afterwards.

## "nostdlib" Motivation & Purpose

nostdlib is a lightweight, minimalistic approach to C development that removes dependencies on the standard library. The motivation behind this project is to provide developers with greater control over their code by eliminating unnecessary overhead, reducing binary size, and enabling deployment in resource-constrained environments.

Many modern development environments rely heavily on the standard library, which, while convenient, introduces unnecessary bloat, security risks, and unpredictable dependencies. nostdlib aims to give developers fine-grained control over memory management, execution flow, and system calls by working directly with the underlying platform.

### Benefits

#### Minimal overhead
By removing the standard library, nostdlib significantly reduces runtime overhead, allowing for faster execution and smaller binary sizes.

#### Increased security
Standard libraries often include unnecessary functions that increase the attack surface of an application. nostdlib mitigates security risks by removing unused and potentially vulnerable components.

#### Reduced binary size
Without linking to the standard library, binaries are smaller, making them ideal for embedded systems, bootloaders, and operating systems where storage is limited.

#### Enhanced performance
Direct control over system calls and memory management leads to performance gains by eliminating abstraction layers imposed by standard libraries.

#### Better portability
By relying only on fundamental system interfaces, nostdlib allows for easier porting across different platforms without worrying about standard library availability.
