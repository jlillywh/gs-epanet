# Contributing to EPANET-GoldSim Bridge

Thank you for your interest in contributing! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/gs-epanet.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Test your changes
6. Commit with clear messages
7. Push to your fork
8. Submit a Pull Request

## Development Setup

### Prerequisites
- Windows 10/11 (64-bit)
- Visual Studio 2022 or 2026 with C++ development tools
- Git for Windows

### Building
```powershell
.\build.ps1 -Configuration Release -Platform x64
```

### Running Tests
```powershell
cd tests
.\run_tank_level_test.ps1
```

## Code Style

- Follow existing code style and conventions
- Use meaningful variable and function names
- Add comments for complex logic
- Update documentation when adding features

## Testing

- All new features must include tests
- Ensure existing tests pass before submitting PR
- Add test cases for bug fixes

## Documentation

- Update README.md for user-facing changes
- Update CHANGELOG.md following Keep a Changelog format
- Add inline code comments for complex implementations

## Pull Request Process

1. Update CHANGELOG.md with your changes
2. Ensure all tests pass
3. Update documentation as needed
4. Provide clear PR description explaining:
   - What changed
   - Why it changed
   - How to test it

## Reporting Issues

When reporting issues, please include:
- Operating system and version
- Visual Studio version
- GoldSim version
- Steps to reproduce
- Expected vs actual behavior
- Relevant log files (`epanet_bridge_debug.log`)

## Questions?

Feel free to open an issue for questions or discussions.

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
