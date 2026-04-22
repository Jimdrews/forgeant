# Contributing to Forgeant

Thanks for your interest in contributing. Here's how to get involved.

## Reporting Bugs

Open a [bug report](https://github.com/Jimdrews/forgeant/issues/new?template=bug_report.yml) with:

- What you expected to happen
- What actually happened
- Steps to reproduce
- Compiler, OS, and forgeant version

## Suggesting Features

Open a [feature request](https://github.com/Jimdrews/forgeant/issues/new?template=feature_request.yml) describing the problem you're trying to solve and your proposed approach.

## Pull Requests

1. Fork the repo and create a branch from `main`
2. Make your changes
3. Ensure all tests pass and formatting is clean
4. Open a PR with a clear description of what changed and why

## Development Setup

```bash
git clone https://github.com/Jimdrews/forgeant.git
cd forgeant
cmake --preset dev
cmake --build --preset dev
```

## Running Tests

```bash
ctest --preset dev
```

## Code Style

- Code is formatted with `clang-format-19` (config in `.clang-format`)
- Static analysis runs via `clang-tidy-19`
- Both are enforced in CI -- run them locally before pushing:

```bash
find include src tests -name '*.hpp' -o -name '*.cpp' | xargs clang-format-19 -i
```
