# GitHub Configuration for FileVault

This directory contains all configuration, documentation, and instructions for the FileVault project.

## 📁 Directory Structure

```
.github/
├── README.md                        # This file
├── copilot-instructions.md          # Instructions for GitHub Copilot
├── ARCHITECTURE.md                  # System architecture documentation
├── SECURITY_GUIDELINES.md           # Security best practices
├── BOTAN3_REFERENCE.md             # Botan 3.x API reference
├── TROUBLESHOOTING.md              # Common issues and solutions
├── PROJECT_CHECKLIST.md            # Development checklist
│
└── workflows/                       # GitHub Actions CI/CD
    ├── build.yml                    # Build and test
    ├── security.yml                 # Security scanning
    └── release.yml                  # Release automation
```

---

## 📋 Documentation Index

### For GitHub Copilot

📄 **[copilot-instructions.md](copilot-instructions.md)**
- Coding standards (C++20, Botan 3.x)
- Security rules (CRITICAL!)
- Algorithm implementation templates
- Testing requirements
- Code review checklist

**Use this when:** Writing any code, implementing algorithms, or reviewing PRs.

---

### For Developers

📐 **[ARCHITECTURE.md](ARCHITECTURE.md)**
- System architecture diagrams
- Component interactions
- Data flow documentation
- File format specification
- Extension points

**Use this when:** Understanding the system, designing new features, or onboarding.

---

🔒 **[SECURITY_GUIDELINES.md](SECURITY_GUIDELINES.md)**
- Critical security rules
- Common vulnerabilities
- Testing requirements
- Best practices
- Security checklist

**Use this when:** Implementing crypto, handling passwords, or security review.

---

📚 **[BOTAN3_REFERENCE.md](BOTAN3_REFERENCE.md)**
- Botan 3.x API guide
- Common operations
- Code examples
- Migration from 2.x
- Performance tips

**Use this when:** Using Botan API, implementing algorithms, or troubleshooting.

---

🔧 **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)**
- Common build issues
- Runtime problems
- Security issues
- Performance problems
- Debugging tools

**Use this when:** Something doesn't work, debugging, or optimizing.

---

✅ **[PROJECT_CHECKLIST.md](PROJECT_CHECKLIST.md)**
- Development phases
- Feature checklist
- Quality metrics
- Timeline
- Release criteria

**Use this when:** Planning work, tracking progress, or preparing release.

---

## 🚀 Quick Start for Copilot

### Setting Up Copilot

1. **Enable Copilot for this repo:**
   ```bash
   # Copilot will automatically read files in .github/
   # No additional setup needed!
   ```

2. **Priority reading order:**
   1. `copilot-instructions.md` - Coding standards
   2. `SECURITY_GUIDELINES.md` - Security rules
   3. `ARCHITECTURE.md` - System design
   4. `BOTAN3_REFERENCE.md` - Botan API

### Using Instructions

Copilot will automatically:
- Follow C++20 and Botan 3.x patterns
- Generate secure crypto code
- Include proper error handling
- Add comprehensive tests
- Write documentation

**Example prompt:**
```
// @copilot implement AES-256-GCM encryption following security guidelines
// Use unique nonce, proper error handling, and include NIST test
```

---

## 🎯 Development Workflow

### 1. Before Coding

Read relevant documentation:
- Architecture for system design
- Security guidelines for crypto code
- Botan reference for API usage

### 2. While Coding

Follow:
- Copilot instructions for coding standards
- Security guidelines for crypto operations
- Botan reference for API calls

### 3. After Coding

Check:
- Project checklist for completeness
- Security checklist for vulnerabilities
- Troubleshooting for common issues

### 4. Before Committing

Verify:
```bash
# Compile with all warnings
cmake -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror" ..
make

# Run tests
ctest --output-on-failure

# Check memory leaks
valgrind --leak-check=full ./filevault_tests

# Format code
clang-format -i src/**/*.cpp include/**/*.hpp

# Commit with good message
git commit -m "feat: implement AES-256-GCM with NIST vectors"
```

---

## 🔐 Critical Reminders

### Security Rules (MUST FOLLOW!)

1. **NEVER reuse nonces/IVs**
   ```cpp
   // ❌ WRONG
   static std::vector<uint8_t> nonce(12);
   
   // ✅ CORRECT
   Botan::AutoSeeded_RNG rng;
   std::vector<uint8_t> nonce(12);
   rng.randomize(nonce.data(), nonce.size());
   ```

2. **ALWAYS use unique salts**
   ```cpp
   // Generate new salt for EVERY file
   auto salt = generate_random_bytes(32);
   ```

3. **ALWAYS clear sensitive data**
   ```cpp
   Botan::secure_vector<uint8_t> key(32);
   // ... use key ...
   // ~secure_vector() auto-clears
   ```

4. **ALWAYS use constant-time operations**
   ```cpp
   bool valid = Botan::constant_time_compare(a, b, len);
   ```

### Code Quality Rules

1. **One algorithm per file**
2. **Comprehensive documentation**
3. **NIST test vectors for all algorithms**
4. **100% test coverage for crypto code**
5. **No compiler warnings**
6. **Valgrind clean**

---

## 📊 Documentation Status

| Document | Status | Last Updated |
|----------|--------|--------------|
| copilot-instructions.md | ✅ Complete | 2024-11-15 |
| ARCHITECTURE.md | ✅ Complete | 2024-11-15 |
| SECURITY_GUIDELINES.md | ✅ Complete | 2024-11-15 |
| BOTAN3_REFERENCE.md | ✅ Complete | 2024-11-15 |
| TROUBLESHOOTING.md | ✅ Complete | 2024-11-15 |
| PROJECT_CHECKLIST.md | 🔄 In Progress | 2024-11-15 |

---

## 🛠️ Maintenance

### Updating Documentation

When updating any document:

1. Keep `copilot-instructions.md` as source of truth
2. Update `ARCHITECTURE.md` when changing design
3. Add new issues to `TROUBLESHOOTING.md`
4. Keep `BOTAN3_REFERENCE.md` in sync with Botan updates
5. Update `PROJECT_CHECKLIST.md` with progress

### Version Control

All documentation is version controlled:
```bash
# View history
git log -- .github/

# See changes
git diff HEAD~1 .github/copilot-instructions.md
```

---

## 🤝 Contributing

### For New Contributors

1. **Read this order:**
   - README.md (this file)
   - ARCHITECTURE.md
   - copilot-instructions.md
   - SECURITY_GUIDELINES.md

2. **Setup environment:**
   ```bash
   ./scripts/setup.sh  # or setup.ps1 for Windows
   ```

3. **Run tests:**
   ```bash
   cd build
   ctest --output-on-failure
   ```

4. **Make changes following guidelines**

5. **Submit PR with description**

### For Maintainers

Update documentation when:
- Adding new features
- Changing architecture
- Discovering new issues
- Updating dependencies
- Security advisories

---

## 📞 Getting Help

### Documentation Issues

If documentation is:
- **Unclear**: Open issue with label `documentation`
- **Outdated**: Open PR with updates
- **Missing**: Open issue requesting addition

### Technical Issues

1. Check `TROUBLESHOOTING.md`
2. Search GitHub issues
3. Ask in Discussions
4. Create new issue with template

---

## 🎓 Learning Path

### New to Cryptography?

1. Read `SECURITY_GUIDELINES.md`
2. Study `BOTAN3_REFERENCE.md` examples
3. Review `ARCHITECTURE.md` for design
4. Start with simple algorithms (hash functions)
5. Progress to symmetric encryption
6. Master key derivation
7. Advanced: post-quantum crypto

### New to C++20?

1. Review `copilot-instructions.md` for C++20 patterns
2. Study existing code
3. Read "Effective Modern C++"
4. Practice with small features
5. Ask for code review

### New to Botan?

1. Start with `BOTAN3_REFERENCE.md`
2. Try simple examples (hashing)
3. Progress to encryption
4. Study test cases
5. Reference official docs

---

## 📈 Project Status

**Current Phase:** Phase 1 - Core Infrastructure (40%)

**Next Milestone:** Complete symmetric encryption algorithms

**Latest Updates:**
- ✅ Project structure created
- ✅ Build system configured
- ✅ Documentation completed
- 🔄 Core engine in progress
- 📅 Algorithms planned

**See:** `PROJECT_CHECKLIST.md` for detailed status

---

## 🌟 Best Practices

### When Writing Code

1. ✅ Read relevant docs first
2. ✅ Follow security guidelines
3. ✅ Write tests alongside code
4. ✅ Document as you go
5. ✅ Review your own PR first

### When Reviewing Code

1. ✅ Check against security guidelines
2. ✅ Verify tests exist and pass
3. ✅ Ensure documentation present
4. ✅ Look for common pitfalls
5. ✅ Test locally

### When Debugging

1. ✅ Check `TROUBLESHOOTING.md` first
2. ✅ Use debug tools (Valgrind, gdb)
3. ✅ Add logging if needed
4. ✅ Write test to reproduce
5. ✅ Document solution

---

## 🔍 Quick Reference

### Most Important Files

For coding: `copilot-instructions.md`, `SECURITY_GUIDELINES.md`
For design: `ARCHITECTURE.md`
For API: `BOTAN3_REFERENCE.md`
For issues: `TROUBLESHOOTING.md`
For planning: `PROJECT_CHECKLIST.md`

### Most Important Rules

1. **Unique nonce per encryption**
2. **Unique salt per file**
3. **Clear sensitive data**
4. **Constant-time comparison**
5. **NIST test vectors**

### Most Common Issues

1. Botan not found → See `TROUBLESHOOTING.md` #1
2. Nonce reuse → See `SECURITY_GUIDELINES.md` Rule #1
3. Memory leaks → Use `secure_vector`
4. Wrong key size → Check algorithm requirements

---

## 📅 Update Schedule

This documentation is reviewed:
- **Weekly**: During active development
- **Monthly**: During maintenance
- **Quarterly**: For major updates
- **As needed**: For security issues

**Last Review:** 2024-11-15
**Next Review:** 2024-11-22

---

## ✅ Checklist for New Features

Before implementing a feature:

- [ ] Read relevant documentation
- [ ] Design follows architecture
- [ ] Security reviewed
- [ ] Tests planned
- [ ] Documentation updated
- [ ] Copilot instructions cover it

---

**Questions?** Open a GitHub Discussion or Issue.

**Found an error?** Submit a PR with the fix.

**Want to contribute?** Read `CONTRIBUTING.md` (coming soon).

---

*This documentation is actively maintained. Last updated: 2024-11-15*