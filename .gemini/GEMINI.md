# lwNBD Project Context & Guidelines

## 1. Project Overview
- **Name:** lwNBD (Lightweight Network Block Device).
- **Architecture:** Decouples protocols (Servers) from drivers (Plugins) via Contexts.
- **Target:** Prototyping on Linux (primary), deployment on PS2/Embedded.
- **Vision:** A modular super-server framework (similar to xinetd/systemd).

## 2. Technical Constraints
- **Language:** C99, strictly NO dynamic memory allocation (`malloc`/`free`).
- **OOP Pattern:** Follows "Quantum Leaps" (Miro Samek) style.
    - Structure nesting for inheritance (Base class must be the first member).
    - V-Tables for polymorphism.
    - Mandatory use of the `me` (or `self`) pointer as the first argument in plugin/server methods.

## 3. Server Architecture (The xinetd Model)
- **Directory:** `servers/<protocol>/`.
- **Core Engine:** `src/servers.c` handles the network lifecycle (socket, bind, listen, accept).
- **Service Contract:** - Servers are defined by a `lwnbd_server_t` structure.
    - Servers implement a simple `handler(client_fd, context)` callback.
- **Encapsulation:** All instance-specific data (ports, custom state) resides in the derived server structure.
- **Registration:** Uses `NBDKIT_REGISTER_SERVER()` for discovery via `get_server_by_name()`.

## 4. Context & Dispatching
- **Dispatcher:** `lwnbd_get_context(name)` maps an export name to a `lwnbd_context_t`.
- **Handle Management:** `src/context.c` manages the `me` pointers for plugins, abstracting them from the user.
- **Command Bridge:** `lwnbd_context_exec_command()` allows servers (HTTP/Shell) to invoke plugin-specific logic via string commands.

## 5. Network Strategy
- **Current State:** Using standard Berkeley sockets within the Core for stability on Linux.
- **Roadmap:** Transitioning to a multiplexed asynchronous model using `select()` in `src/servers.c`.
- **Abstraction Goal:** Future integration of `include/lwnbd/tcp.h` to decouple protocol logic from the network stack (supporting TCP/UDP/Multicast SSDP).

## 6. Development Workflow
- **Philosophy:** Keep handlers extremely thin. Protocol logic only.
- **Example Service:** "Daytime" (RFC 867) serves as the reference stateless implementation.
- **Next Target:** HTTP RPC-style server using `yuarel` for URI routing (`/context/command`).