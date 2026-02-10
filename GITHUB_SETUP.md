# GitHub Setup Guide

This guide will help you push the EPANET-GoldSim Bridge project to GitHub for the first time.

## Pre-Push Checklist

✅ **Version**: Updated to 1.0.0  
✅ **Build**: Successful (Release x64)  
✅ **Tests**: All passing (tank level validation)  
✅ **Documentation**: Complete and user-focused  
✅ **.gitignore**: Configured to exclude build artifacts and IDE files  
✅ **License**: MIT License included  

## Step 1: Initialize Git Repository

If you haven't already initialized git in your project directory:

```bash
cd C:\Users\JasonLillywhite\source\repos\gs_epanet
git init
```

## Step 2: Add Remote Repository

Add your GitHub repository as the remote origin:

```bash
git remote add origin https://github.com/jlillywh/gs-epanet.git
```

Verify the remote was added:

```bash
git remote -v
```

## Step 3: Stage All Files

Add all files to the staging area (respecting .gitignore):

```bash
git add .
```

Check what will be committed:

```bash
git status
```

## Step 4: Create Initial Commit

Commit with a descriptive message:

```bash
git commit -m "Initial release v1.0.0

- GoldSim External Function API implementation
- JSON-based configuration system
- Pump control, valve control, emitter-based flow control
- Real-time monitoring of pressures, flows, tank levels
- Validated tank level dynamics
- Pump_to_Tank working example with deadband controller
- Comprehensive documentation and tests
- EPANET 2.3.3 integration"
```

## Step 5: Push to GitHub

Set the default branch to main and push:

```bash
git branch -M main
git push -u origin main
```

If you encounter authentication issues, you may need to use a Personal Access Token (PAT) instead of a password.

## Step 6: Configure GitHub Repository Settings

After pushing, configure your repository on GitHub:

### Repository Description

```
Windows DLL bridge connecting GoldSim simulation software with EPA EPANET hydraulic modeling for real-time water distribution system control
```

### Topics (Tags)

Add these topics to help others discover your project:
- `epanet`
- `goldsim`
- `water-distribution`
- `hydraulic-modeling`
- `simulation`
- `water-resources`
- `pump-control`
- `windows`
- `dll`
- `c-plus-plus`

### About Section

- **Website**: (optional - add if you have documentation site)
- **License**: MIT License (should auto-detect from LICENSE file)

## Step 7: Create a Release (Optional)

To create an official v1.0.0 release:

1. Go to your repository on GitHub
2. Click "Releases" → "Create a new release"
3. Tag version: `v1.0.0`
4. Release title: `v1.0.0 - Initial Stable Release`
5. Description: Copy from CHANGELOG.md
6. Attach binaries (optional):
   - `gs_epanet.dll` (from `bin\x64\Release\`)
   - `epanet2.dll` (from `lib\`)
   - Create a zip with example files

## What Gets Pushed

### ✅ Included Files

**Source Code**:
- `src/` - C++ implementation
- `include/` - Headers (EPANET API, JSON library)
- `lib/` - EPANET libraries and DLLs

**Examples**:
- `examples/Pump_to_Tank/` - Complete working example
  - EPANET network file
  - JSON configuration
  - Screenshots

**Tests**:
- `tests/` - Validation tests and test scripts

**Documentation**:
- `README.md` - User guide
- `CHANGELOG.md` - Version history
- `CONTRIBUTING.md` - Contribution guidelines
- `LICENSE` - MIT License
- `docs/reference/` - API reference documents

**Build System**:
- `EpanetGoldSimBridge.sln` - Visual Studio solution
- `src/EpanetBridge.vcxproj` - Project file
- `build.ps1` - Build script
- `create_distribution.ps1` - Distribution script

### ❌ Excluded Files (via .gitignore)

**Build Artifacts**:
- `bin/` - Compiled DLLs
- `obj/` - Object files
- `*.pdb` - Debug symbols
- `*.ilk` - Incremental link files

**IDE Settings**:
- `.vs/` - Visual Studio cache
- `.vscode/` - VS Code settings
- `.kiro/` - Kiro IDE settings

**Runtime Files**:
- `*.log` - Debug logs
- `*.rpt` - EPANET reports
- `*.out` - EPANET output files

**Distribution**:
- `EPANET_Bridge_v*/` - Distribution folders
- `*.zip` - Archive files

## Troubleshooting

### Authentication Failed

If you get authentication errors:

1. **Use Personal Access Token (PAT)**:
   - Go to GitHub Settings → Developer settings → Personal access tokens
   - Generate new token with `repo` scope
   - Use token as password when prompted

2. **Configure Git Credential Manager**:
   ```bash
   git config --global credential.helper manager-core
   ```

### Large Files Warning

If you get warnings about large files:
- Check that `bin/` and `obj/` are properly excluded
- Verify `.gitignore` is working: `git check-ignore -v <filename>`

### Wrong Files Being Committed

If unwanted files are staged:
```bash
git reset HEAD <filename>  # Unstage specific file
git reset HEAD .           # Unstage all files
```

Then update `.gitignore` and re-add files.

## Post-Push Tasks

After successfully pushing to GitHub:

1. ✅ Verify all files are visible on GitHub
2. ✅ Check that README.md displays correctly with images
3. ✅ Verify LICENSE is detected
4. ✅ Add repository description and topics
5. ✅ Create v1.0.0 release (optional)
6. ✅ Add GitHub Actions for CI/CD (optional, future enhancement)

## Future Updates

For subsequent updates:

```bash
# Make your changes
git add .
git commit -m "Description of changes"
git push

# For new versions, update:
# 1. src/EpanetBridge.cpp (XF_REP_VERSION)
# 2. CHANGELOG.md
# 3. README.md (version section)
# 4. Create GitHub release
```

## Getting Help

If you encounter issues:
- Check GitHub's documentation: https://docs.github.com
- Verify .gitignore is working: `git status`
- Review what will be committed: `git diff --cached`

---

**Ready to push?** Follow Steps 1-5 above to publish your project to GitHub! 🚀
