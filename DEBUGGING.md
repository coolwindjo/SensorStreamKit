# Debugging SensorStreamKit

## Issue Summary

Interactive debugging with breakpoints (LLDB/GDB) **does not work in Docker Desktop on macOS** due to fundamental ptrace restrictions in the Docker Desktop linuxkit kernel.

## Root Cause

Docker Desktop on Mac runs containers inside a minimal Linux VM (linuxkit) that has security restrictions preventing debuggers from:
- Setting breakpoints in process memory
- Reading/writing CPU registers
- Controlling process execution (step, continue)

**Technical Details:**
- Error with LLDB: `failed to set breakpoint site... error: 9`
- Error with GDB: `Cannot PTRACE_GETREGS: Input/output error`
- Present even with `CAP_SYS_PTRACE` and `seccomp:unconfined` configured
- Limitation of Docker Desktop's security model, not fixable from inside container

## Workarounds

### Option 1: Debug on macOS Native (Recommended)

Use native macOS for all interactive debugging:

```bash
# On your MacBook terminal
cd ~/path/to/SensorStreamKit
rm -rf build  # Clean any Docker builds
cmake --preset debug
cmake --build build

# In VSCode:
# 1. Install CodeLLDB extension
# 2. Open tests/test_message_class.cpp
# 3. Set breakpoint
# 4. Press F5 → Select "LLDB: Debug Tests (macOS)"
# Result: Breakpoints work perfectly ✅
```

**VSCode Configuration:**
The `.vscode/launch.json` includes LLDB configurations for macOS:
- `LLDB: Debug Tests (macOS)` - Run all tests
- `LLDB: Debug Specific Test (macOS)` - Run with GTest filter

**Requirements:**
- Install **CodeLLDB** extension in VSCode
- LLDB comes with Xcode Command Line Tools: `xcode-select --install`

### Option 2: Use Native Linux Machine

If you have access to a native Linux machine (not Docker Desktop):
- Both LLDB and GDB work normally with breakpoints
- Use GDB configurations in `.vscode/launch.json`:
  - `GDB: Debug Tests (Linux)` - Run all tests
  - `GDB: Debug Specific Test (Linux)` - Run with GTest filter

**Requirements:**
- Install GDB: `sudo apt install gdb` (Ubuntu/Debian)
- Build with debug symbols: `cmake --preset debug && cmake --build build`

### Option 3: Log-Based Debugging in Docker

When you must debug in Docker Desktop, use logging:

```cpp
#include <iostream>

// Add debug output
std::cout << "DEBUG: Line 361, buffer size = " << buffer.size() << std::endl;

// Or use GTest assertions with messages
EXPECT_TRUE(condition) << "Debug info: value=" << value;
```

## Platform Compatibility

| Platform | Build | Debug with Breakpoints | Debugger | Configuration |
|----------|-------|----------------------|----------|---------------|
| **macOS (native)** | ✅ | ✅ | LLDB | `LLDB: Debug Tests (macOS)` |
| **Docker Desktop (Mac)** | ✅ | ❌ | N/A | Use macOS native |
| **Native Linux** | ✅ | ✅ | GDB/LLDB | `GDB: Debug Tests (Linux)` |
| **Docker on Linux** | ✅ | ✅ | GDB | `GDB: Debug Tests (Linux)` |

## Recommended Workflow

### Daily Development
```bash
# macOS native - full debugging capability
cd ~/SensorStreamKit
cmake --preset debug && cmake --build build
# Debug with breakpoints in VSCode
```

### Before Committing
```bash
# Docker - verify Linux compatibility
docker compose up -d dev
docker exec sensorstreamkit-dev bash -c "
  cd /workspace &&
  rm -rf build &&
  cmake --preset debug &&
  cmake --build build &&
  ./build/tests/test_message_class
"
```

## Important Notes

### Binary Format Differences

The `build/` directory contains platform-specific binaries:
- **macOS:** Mach-O ARM64 format
- **Linux/Docker:** ELF x86-64 format

**Always rebuild when switching platforms:**
```bash
rm -rf build  # Clear old binaries
cmake --preset debug && cmake --build build
```

### Docker Configurations

The dev container in `docker/docker-compose.yml` has debugging capabilities configured:
```yaml
cap_add:
  - SYS_PTRACE
security_opt:
  - seccomp:unconfined
```

These are necessary but **not sufficient** for debugging in Docker Desktop on Mac. They work correctly on native Linux.

## Future Work

Potential improvements if Docker Desktop limitations are lifted:

1. **If Docker Desktop adds debugging support:**
   - Re-enable LLDB configurations for Docker
   - Test with newer Docker Desktop versions
   - Update this documentation

2. **Alternative solutions to explore:**
   - Remote debugging from macOS to Docker container
   - Use Lima or Podman as Docker Desktop alternative
   - Cloud-based Linux development environment

3. **Current best practice:**
   - Use macOS native for all interactive debugging
   - Use Docker for builds, tests, and CI/CD
   - Document platform-specific behavior in code comments

## Quick Reference

**For debugging with breakpoints:**
- macOS: Use `LLDB: Debug Tests (macOS)` configuration
- Linux: Use `GDB: Debug Tests (Linux)` configuration
- Docker Desktop on Mac: ❌ Not supported - use native macOS

**For building/testing:** Use macOS native or Docker
**For CI/CD:** Use Docker
**For Linux compatibility:** Build and test in Docker (without debugging)

## VSCode Debug Configurations

Available in `.vscode/launch.json`:

1. **GDB: Debug Tests (Linux)** - For native Linux or Docker on Linux
2. **GDB: Debug Specific Test (Linux)** - Run specific GTest with filter
3. **LLDB: Debug Tests (macOS)** - For native macOS debugging
4. **LLDB: Debug Specific Test (macOS)** - Run specific GTest on macOS

---

**Last Updated:** December 2024
**Status:** Docker Desktop debugging not possible - use native macOS
