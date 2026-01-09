[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/bignaux/lwNBD)


lwNBD(3) -- A Lightweight software component framework
=============================================

## 👇 SYNOPSIS

    #include <lwnbd/lwnbd.h>
    #include <lwnbd/lwnbd-server.h>
    #include <lwnbd/lwnbd-plugin.h>

## ✨ DESCRIPTION

Universal Embedded Service Framework (UESF)

lwNBD (transitioning to a generic service framework) is a lightweight, zero-allocation static library designed to provide consistent management and connectivity services across heterogeneous embedded environments.
The Problem: Firmware & OS Inconsistency

In the embedded and retro-computing world, developers often face fragmented environments where essential services (remote debugging, file transfer, configuration shells) are either missing, inconsistent, or tied to specific OS kernels.
The Solution: Application-Level Infrastructure

Instead of relying on the host OS or firmware to provide tools, this framework allows your application to carry its own infrastructure. By linking the library, your binary gains:

    Protocol Independence: Start with NBD for block I/O, but easily plug in HTTP RPC, custom Command Shells, or GDB-like stubs.

    Hardware Abstraction: Expose any internal resource (Raw Memory, UART, Flash, Block Storage) through a unified "Context/Plugin" architecture.

    Total Determinism: Designed for hard real-time constraints with a strict no-malloc policy, ensuring a predictable memory footprint regardless of the uptime.

Key Philosophy

    Linkable, not Standalone: Designed to be integrated into monolithic binaries.

    Platform Agnostic: Runs on anything from a 32-bit microcontroller to a full Linux system.

    Contributor Friendly: The core handles the "networking plumbing" (socket management, multiplexing), allowing contributors to focus solely on implementing new Protocols (Servers) or Drivers (Plugins).

* Official repository : <https://github.com/bignaux/lwNBD>
* BSD-style socket API: lwIP 2.0.0 and Linux supported.

Targeting first the use on Playstation 2 IOP, a 37.5 MHz MIPS processor
and 2 MB of RAM, lwNBD is designed to run on bare metal or OS embedded system.
With modulararity and portability in mind, it is developed according to several
code standards, including :

* no dynamically allocate memory
* static-linking
* written in C99 (using -std=c99 compile-time flag), use standard library usage
* thread-safe synchronous NBD protocol implementation
* optional and experimental query support 

The lwNBD API is broken down into 3:

* an API to manage server and content to be embbed in apps. The idea is to be able to manage servers as xinetd would.

* a server API to create protocol and transport plugins. Currently, only support [NBD protocol](./servers/nbd/lwnbd-nbd-server.md), but could extend to Zmodem, AoE ... You then benefit from the mechanisms put in place for NBD such as content and management plugins.

* a plugin API to create content plugins. The plugin API has been entirely rewritten to be closer to the [nbdkit-plugin](https://libguestfs.org/nbdkit-plugin.3.html) one in order to benefit from the experience of their software architecture, and to facilitate the porting of code from one or the other library. An obstacle to this convergence is that nbdkit does not support the use of multiple plugins in a single instance.


## Why lwnbd and not Mongoose?

While general-purpose libraries like Mongoose provide excellent connectivity, lwNBD is engineered for hard real-time constraints and deterministic embedded systems (e.g., retro-consoles like the PS2, or RTOS-based microcontrollers).
1. Determinism vs. Flexibility

    Mongoose relies on malloc() for internal buffering and state management. This introduces non-deterministic timing and potential heap fragmentation, which can be fatal in critical systems.

    lwNBD follows a Strictly No Malloc policy. All resources, server instances, and buffers are pre-allocated at compile time or startup. This ensures O(1) performance and prevents runtime "Out of Memory" crashes due to fragmentation.

2. Kconfig-Driven Modularity

    Mongoose is often distributed as a "monolith" (amalgamated source). Disabling features requires manual preprocessor defines.

    lwNBD leverages the Kconfig system (as seen in the Linux Kernel). This allows for deep, dependency-aware pruning of the codebase. You compile only the code you need, resulting in a minimal binary footprint tailored specifically for your hardware.

3. Super-Server (xinetd) Architecture

    Unlike typical libraries where the user manages the network loop, lwNBD acts as a Service Runner. The Core handles the socket lifecycle (binding, listening, and multiplexing), while services (NBD, HTTP, Daytime) are implemented as simple, stateless Handlers.

4. Real-Time Safety

    By eliminating dynamic memory, lwNBD satisfies high-reliability standards (similar to MISRA C guidelines). It is designed to run indefinitely without memory leaks, making it the ideal choice for systems where reliability and predictable latency are paramount.

|Feature         |Mongoose                               |lwNBD                                                        |
|----------------|---------------------------------------|-------------------------------------------------------------|
|Core Philosophy |General-purpose library.               |Modular "Super-Server" Framework.                            |
|Configuration   |C Preprocessor macros (manual).        |Kconfig-based (visual & dependency-aware).                   |
|Memory Policy   |Uses malloc for buffers/internal state.|Strictly No Malloc (static allocation only).                 |
|Modularization  |Monolithic (single file).              |Micro-kernel style (strict separation: Core/Plugins/Servers).|
|NBD Native      |No (requires custom protocol logic).   |Native & Optimized (core feature).                           |
|Footprint Tuning|Difficult to exclude core logic.       |Compile-time pruning of every unused module.                 |

## 📜 HISTORY

On Playstation 2, there is no standardized central partition table like GPT for
hard disk partitioning, nor is there a standard file system but PFS and
HDLoader. In fact, there are few tools capable of handling hard disks,
especially under Linux, and the servers developed in the past to handle these
disks via the network did not use a standard protocol, which required each
software wishing to handle the disks to include a specific client part,
which were broken when the toolchain was updated. The same goes for the memory
cards and other block devices on this console, which is why I decided to
implement NBD on this target first.

## 🪶 AUTHOR

Bignaux Ronan &lt;rbignaux at free.fr&gt;

## ©️ LICENSE

BSD

