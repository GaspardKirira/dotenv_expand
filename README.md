# dotenv_expand

Deterministic dotenv-style variable expansion for modern C++.

`dotenv_expand` provides predictable environment variable interpolation compatible with common `.env` expansion behavior:

- `${VAR}`
- `$VAR`
- `${VAR:-default}`

Header-only. Zero external dependencies.

## Download

https://vixcpp.com/registry/pkg/gaspardkirira/dotenv_expand

## Why dotenv_expand?

Environment-based configuration appears everywhere:

- API URLs built from host + port
- Database credentials
- Service discovery endpoints
- Docker / Kubernetes configs
- CI/CD pipelines
- Cross-platform tooling

Typical problems when expanding manually:

- Incorrect nested resolution
- Silent infinite recursion
- Platform differences (Windows vs Linux)
- Missing default handling
- Inconsistent behavior across environments

This library provides:

- Deterministic expansion
- Nested variable resolution
- Default value syntax `${VAR:-default}`
- Safe cycle detection
- Optional process environment fallback
- Windows-safe environment lookup
- Header-only simplicity

No file parsing.
No global state.
No hidden behavior.

Just explicit expansion primitives.

## Installation

### Using Vix Registry

```bash
vix add gaspardkirira/dotenv_expand
vix deps
```

### Manual

```bash
git clone https://github.com/GaspardKirira/dotenv_expand.git
```

Add the `include/` directory to your project.

### Dependency

Requires C++17 or newer. No external dependencies.

## Quick examples

### Basic expansion

```cpp
#include <dotenv_expand/dotenv_expand.hpp>
#include <iostream>
#include <unordered_map>

int main()
{
    std::unordered_map<std::string, std::string> env{
        {"HOST", "localhost"},
        {"PORT", "8080"},
        {"API_URL", "http://${HOST}:${PORT}"}
    };

    dotenv_expand::expand_inplace(env);

    std::cout << env["API_URL"] << "\n";
}
```

Output:

```
http://localhost:8080
```

### Nested variables

```cpp
#include <dotenv_expand/dotenv_expand.hpp>
#include <iostream>
#include <unordered_map>

int main()
{
    std::unordered_map<std::string, std::string> env{
        {"HOST", "127.0.0.1"},
        {"PORT", "3000"},
        {"BASE", "http://${HOST}:${PORT}"},
        {"V1", "${BASE}/v1"}
    };

    dotenv_expand::expand_inplace(env);

    std::cout << env["V1"] << "\n";
}
```

### Default values

```cpp
#include <dotenv_expand/dotenv_expand.hpp>
#include <iostream>
#include <unordered_map>

int main()
{
    std::unordered_map<std::string, std::string> env{
        {"HOST", "example.com"}
    };

    auto url = dotenv_expand::expand(
        "https://${HOST}:${PORT:-443}/api",
        env
    );

    std::cout << url << "\n";
}
```

### Using process environment

```cpp
#include <dotenv_expand/dotenv_expand.hpp>
#include <iostream>

int main()
{
    std::cout << dotenv_expand::expand_env(
        "Hello ${USER:-Anonymous}"
    ) << "\n";
}
```

### Escaping `$`

```cpp
#include <dotenv_expand/dotenv_expand.hpp>
#include <iostream>
#include <unordered_map>

int main()
{
    std::unordered_map<std::string, std::string> env{
        {"X", "VALUE"}
    };

    auto s = dotenv_expand::expand(
        R"(Price=100\$ var=${X})",
        env
    );

    std::cout << s << "\n";
}
```

Output:

```
Price=100$ var=VALUE
```

## API overview

- `dotenv_expand::expand(input, vars, options);`
- `dotenv_expand::expand_env(input, options);`
- `dotenv_expand::expand_inplace(vars, options);`

### Options

```cpp
struct Options
{
    bool allow_process_env = true;
    int  max_passes = 16;
};
```

## Expansion rules

Resolution order:

1. Provided variable map
2. Process environment (if enabled)

Unknown variables expand to empty string.

Nested expansion is supported.

Cycles are detected via convergence limit.

## Complexity

Let:

- `N` = input string length
- `P` = number of passes (bounded, default 16)

| Operation             | Time complexity |
|----------------------|-----------------|
| Single expansion pass | O(N)            |
| Full expansion        | O(P × N)        |

Since `P` is bounded, runtime remains deterministic.

## Semantics

- `${VAR}` expands to value or empty string
- `$VAR` expands to value or empty string
- `${VAR:-default}` uses default when variable is empty or missing
- `\$` produces a literal `$`
- Expansion is stable and deterministic
- Convergence is enforced (cycle-safe)

## Design principles

- Deterministic behavior
- Explicit over implicit
- No global state
- No hidden caching
- Cross-platform correctness
- Windows-safe environment lookup
- Header-only simplicity

This library expands values only.

If you need:

- Full `.env` file parsing
- Advanced shell expression parsing
- Conditional logic
- Export semantics

Build those layers on top of this primitive.

## Tests

Run:

```bash
vix build
vix test
```

Tests verify:

- Braced and unbraced expansion
- Nested resolution
- Default values
- Escape behavior
- Cycle detection
- Cross-platform environment lookup

## License

MIT License\
Copyright (c) Gaspard Kirira

