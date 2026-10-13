# Coding conventions

These conventions apply to project-owned C++ code. Preserve generated and vendored code as supplied. Module responsibilities and application flow belong in [project structure](project-structure.md); build commands and verification belong in [development](development.md).

## Language and style

- Use C++17 and the standard library. Prefer facilities that express intent and ownership; a header's age alone does not make it obsolete.
- Use four spaces, no tabs. Put function braces on the next line and class/control-flow braces on the same line. Use braces for control-flow bodies, including single statements.
- Use `PascalCase` for types and their filenames, and `camelCase` for functions and variables. Private data members use a trailing underscore, such as `window_`; private methods use ordinary `camelCase` without a suffix. Keep `main.cpp` lowercase.
- Use `nullptr`, `const`, and `constexpr` where appropriate. Use `auto` when the type is clear from the expression; otherwise spell it out.
- Use no project namespace for now. Keep implementation-only free functions and constants in anonymous namespaces. Never put `using namespace` directives in headers.
- Keep headers self-contained, with `#pragma once`, and place them beside their `.cpp` files. Include the matching header first in implementations, then required standard, third-party, and other project headers.
- Include the header that declares each library facility used; do not rely on transitive includes. Ordinary `new`/`delete` expressions and `= delete` declarations are language features. Use `<memory>` for smart pointers and `<exception>` for `std::exception`.
- Comments explain intent, lifetime constraints, units, or temporary behavior. Avoid narrating obvious statements.

## Ownership and resource lifetime

- Prefer values and direct members. Allocate dynamically when lifetime or another concrete requirement calls for it.
- Use `std::unique_ptr` and `std::make_unique` for exclusive heap ownership. Use `std::shared_ptr` only when ownership is actually shared; document the owners.
- Avoid owning raw `new`/`delete` pairs. Raw pointers and references normally borrow objects; document their required lifetime. Prefer references when null is not a valid state.
- C APIs can require raw owning handles. Encapsulate these in a resource owner with a destructor or a smart pointer with the correct deleter. Match native handles with their API's release function, never ordinary `delete`.
- Disable copying for resource owners. Add move support only when needed, transferring the handle and clearing the source. Prefer the rule of zero for ordinary data objects.
- Order owning members so dependencies outlive their users. Destructors must tolerate partial initialization and must not throw.
- Keep native handles private. Restrict unavoidable native access to platform integration; do not add a public handle getter for convenience.
- At C API boundaries, document any ownership transfer explicitly. Release a smart pointer only when the receiving API requires it and there is a matching cleanup path.

## Initialization and errors

- Keep platform/resource constructors empty and establish fallible resources through explicit initialization. Initialization is a one-time operation unless repeat initialization is deliberately supported.
- Check external API return values. Log the failed operation and its diagnostic, then propagate failure to the caller.
- Never allow C++ exceptions to unwind through C callbacks. Convert exceptions into the API's error result at callback boundaries that perform throwing operations.
- Keep cleanup automatic. Avoid adding manual cleanup branches for each initialization failure.
- Optional features may degrade gracefully when documented. Do not silently continue after a required initialization step fails.

## Class design

- Give each class a concrete responsibility. Use composition and avoid generic device, manager, singleton, or backend hierarchies without an implementation need.
- Keep public interfaces small and expose operations rather than implementation details.
- Keep independently testable logic separate from external resource access.
- Make coordinate units and conversions explicit.
- Mark temporary/demo code clearly and avoid treating it as a permanent abstraction.
