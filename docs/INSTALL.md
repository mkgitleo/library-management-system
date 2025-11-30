# Installation Guide

## System Requirements
- Windows 10/11 or Linux or Mac
- G++ Compiler (version 8 or higher)
- SQLite3 Development Files
- ~50 MB disk space

## Windows Installation (Detailed)

### Step 1: Install G++ and SQLite3

**Option A: Using MinGW-w64 (Recommended)**
1. Download installer from: https://www.mingw-w64.org/downloads/
2. Run installer, select:
   - Architecture: x86_64
   - Installation folder: `C:\mingw64`
3. Add to PATH:
   - Search "Environment Variables" in Windows
   - Click "Edit the system environment variables"
   - Click "Environment Variables" button
   - Under "System variables", find "Path" and click "Edit"
   - Click "New" and add: `C:\mingw64\bin`
   - Click OK all the way

**Option B: Using Windows Subsystem for Linux (WSL2)**
```powershell
# In PowerShell as Administrator
wsl --install ubuntu

# In Ubuntu terminal
sudo apt update
sudo apt install g++ sqlite3 libsqlite3-dev
```

### Step 2: Download Project
```powershell
# Using Git
git clone https://github.com/YourUsername/library-management-system.git
cd library-management-system

# OR: Download ZIP and extract
```

### Step 3: Compile
```powershell
g++ src/lib_management_sys_sqlite3.cpp -o lib_management.exe -lsqlite3
```

### Step 4: Run
```powershell
.\lib_management.exe
```

---

## Linux Installation

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install g++ sqlite3 libsqlite3-dev git

git clone https://github.com/YourUsername/library-management-system.git
cd library-management-system

g++ src/lib_management_sys_sqlite3.cpp -o lib_management -lsqlite3

./lib_management
```

### Fedora/RHEL
```bash
sudo dnf install gcc-c++ sqlite-devel git

git clone https://github.com/YourUsername/library-management-system.git
cd library-management-system

g++ src/lib_management_sys_sqlite3.cpp -o lib_management -lsqlite3

./lib_management
```

---

## Mac Installation

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install gcc sqlite3 git

git clone https://github.com/YourUsername/library-management-system.git
cd library-management-system

g++ src/lib_management_sys_sqlite3.cpp -o lib_management -lsqlite3

./lib_management
```

---

## Troubleshooting

### Error: "g++ command not found"
**Solution**: G++ not installed or not in PATH
- Reinstall MinGW-w64 and add to PATH (see Step 1)
- Or use WSL2

### Error: "sqlite3.h: No such file"
**Solution**: SQLite3 development files not installed
- Windows: Download from https://www.sqlite.org/download.html
- Linux: `sudo apt install libsqlite3-dev`
- Mac: `brew install sqlite3`

### Error: "library.db permission denied"
**Solution**: Database file permission issue
- Delete `library.db` file and restart
- App will create new database automatically

### Program compiles but won't run
**Solution**: 
- Try running from PowerShell (not Command Prompt)
- Check if sqlite3 DLL is in same folder or PATH

---

## Verification

After successful installation, verify with:
```bash
g++ --version          # Should show G++ version
sqlite3 --version      # Should show SQLite3 version
.\lib_management.exe   # Should show menu
```

If all three work, you're ready to go!
