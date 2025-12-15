# Debugging in Docker Containers

## ⚠️ Important: Docker Desktop on macOS Limitation

Interactive debugging with breakpoints does **NOT** work in Docker Desktop on macOS due to linuxkit kernel ptrace restrictions.

**See [../DEBUGGING.md](../DEBUGGING.md) for:**
- Root cause explanation
- Workarounds
- Recommended workflow

## VSCode Debug Configurations

Available in `../.vscode/launch.json`:

### For Native Linux (Not Docker Desktop)

If using Docker on native Linux, use GDB configurations:

- **GDB: Debug Tests (Linux)** - Run all tests with GDB
- **GDB: Debug Specific Test (Linux)** - Run specific GTest with filter

**Requirements:**
1. Container has proper capabilities (already configured in `docker-compose.yml`):
   ```yaml
   cap_add:
     - SYS_PTRACE
   security_opt:
     - seccomp:unconfined
   ```
2. GDB installed in container
3. Debug build: `cmake --preset debug && cmake --build build`

### For Native macOS

Use LLDB configurations for native macOS debugging (not in Docker):

- **LLDB: Debug Tests (macOS)** - Run all tests with LLDB
- **LLDB: Debug Specific Test (macOS)** - Run specific GTest with filter

**Requirements:**
- CodeLLDB extension in VSCode
- Build on macOS: `cmake --preset debug && cmake --build build`
- Debug natively (not inside Docker container)

## Recommendation

- **macOS users:** Debug natively with LLDB (not in Docker)
- **Linux users:** Debug in Docker with GDB or natively
- **All users:** Use Docker for builds, tests, and CI/CD
