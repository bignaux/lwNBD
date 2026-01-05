# lwNBD Project Context & Guidelines

## 1. Project Overview
- **Name:** lwNBD (Lightweight Network Block Device)
- **Goal:** A modular C library to export block devices via multiple protocols.
- **Target Systems:** Embedded (PlayStation 2, bare metal) and Linux (for prototyping).
- **Core Principle:** Decouple transport protocols (Servers) from hardware drivers (Plugins).

## 2. Technical Constraints
- **Language:** C99.
- **Memory:** Strictly NO dynamic memory allocation (`malloc`, `free`) in core or plugins.
- **Naming:** Follow `lwnbd_` prefix convention for public APIs.
- **Communication:** English is mandatory for code, comments, and technical docs.

## 3. Architecture Patterns
- **Plugins:** Abstract drivers (e.g., `atad`, `file`, `pcmstream`, `command`) implementing I/O callbacks.
- **Contexts:** A context (`lwnbd_context_t`) binds a plugin instance to an export name.
- **Dispatching:** `lwnbd_get_context(name)` is the central dispatcher to find a context by its export name.
- **Servers:** Protocol frontends (NBD, HTTP, Daytime) located in `servers/<protocol>/`.

## 4. Server Plugin Contract
Every server MUST follow this pattern to be integrated into the framework:
- Define a `static lwnbd_server_t` structure.
- Implement `start(lwnbd_context_t *ctx)` as the network loop entry point.
- Register itself using the macro: `NBDKIT_REGISTER_SERVER(server_struct_name)`.

## 5. Current Development State
- **Prototyping:** Actively testing under Linux for fast iteration.
- **Daytime Server:** Being used as a minimal, stateless reference implementation to simplify server-handle management.
- **HTTP Server:** Planned RPC interface using `yuarel` for URI parsing (Route: `/<context>/<command>`).
- **Command Bridge:** `lwnbd_context_exec_command()` is the intended bridge between string-based server requests and plugin logic.

## 6. Directory Structure Convention
- `src/`: Core library logic.
- `plugins/`: Hardware and utility drivers.
- `servers/`: Protocol implementations (one folder per protocol, named after the protocol).

## 7. OOP Implementation (Quantum Leaps Pattern)
- **Pattern:** Following "Object-Oriented Programming in C" (Quantum Leaps).
- **Polymorphism:** Use of structures with function pointer tables (V-Tables) for Plugins.
- **Inheritance:** Achieved through structure nesting (the base class must be the first member of the derived struct).
- **Encapsulation:** Every plugin callback expects a "handle" (the `me` pointer) to maintain its own state without global variables.