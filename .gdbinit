# GDB initialization for SensorStreamKit debugging
# Optimized for Docker/VSCode environment

# Disable terminal control (prevents "Operation not permitted" errors in Docker)
set new-console off

# Disable pagination for non-interactive debugging
set pagination off

# Disable address space randomization (helps with ptrace issues in Docker)
set disable-randomization off

# Enable pretty printing for STL containers
set print pretty on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set demangle-style gnu-v3

# Show full strings
set print elements 0

# Enable history
set history save on
set history filename ~/.gdb_history
set history size 10000

# Auto-load safe path (for pretty printers)
add-auto-load-safe-path /workspace

# Handle ptrace errors gracefully
set breakpoint pending on
set detach-on-fork on
