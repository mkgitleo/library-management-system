# 📚 Library Management System

A **professional C++ desktop application** for complete library operations with SQLite3 database persistence.

<div align="center">

**[Features](#-features)** • **[Installation](#-installation)** • **[Usage](#-usage)** • **[Architecture](#-system-architecture)** • **[Screenshots](#-screenshots)**

</div>

---

## ⭐ Features

### Core Functionality
- ✅ **Book Management** - Add, update, search, delete books
- ✅ **User Management** - Register users, track lending history
- ✅ **Issue/Return Books** - Request and process book issues
- ✅ **Availability Tracking** - Real-time book availability updates
- ✅ **Rating System** - Users can rate books (0-5 stars)
- ✅ **Penalty System** - Automatic penalties for overdue books
- ✅ **History Tracking** - Complete transaction log
- ✅ **Admin Dashboard** - Comprehensive management panel

### Technical Highlights
- **Object-Oriented Design** - Inheritance, polymorphism, encapsulation
- **SQLite3 Database** - Persistent data storage
- **Role-Based Access** - Admin and User modes
- **Cross-Platform** - Works on Windows, Linux, Mac
- **Clean Architecture** - Well-documented, maintainable code

---

## 🚀 Quick Start

### Minimum Requirements
- **OS**: Windows 10+ / Linux / Mac
- **Compiler**: G++ (version 8+)
- **Dependencies**: SQLite3
- **Disk Space**: ~50 MB

### Windows Installation (3 Steps)

```powershell
# 1. Clone repository
git clone https://github.com/YourUsername/library-management-system.git
cd library-management-system

# 2. Build (automatic with build.bat)
.\build.bat

# 3. Run
.\build\lib_management.exe
```

### Linux/Mac Installation

```bash
# Install dependencies
sudo apt install g++ sqlite3 libsqlite3-dev    # Ubuntu/Debian
brew install gcc sqlite3                        # Mac

# Clone and build
git clone https://github.com/YourUsername/library-management-system.git
cd library-management-system

g++ src/lib_management_sys_sqlite3.cpp -o lib_management -lsqlite3

# Run
./lib_management
```

📖 **Detailed instructions**: See [INSTALL.md](docs/INSTALL.md)

---

## 📖 Usage Guide

### Login Credentials

| Role  | Username | Password  |
|-------|----------|-----------|
| Admin | (any)    | admin123  |
| User  | Any      | No auth   |

### Admin Features

```
1. Add Book              → Add new book to library
2. View Books           → List all books
3. Issue Book Request   → Approve pending requests
4. Return Book Request  → Process returns
5. Add User             → Register new member
6. View Users           → List all members
7. View Defaulters      → See penalty list
8. View History (N)     → Transaction records
9. Save                 → Manual database save
```

### User Features

```
1. View Books           → Browse available books
2. Issue Book           → Request book (1 per user max)
3. Return Book          → Return borrowed book
4. Check Status         → View your borrowing status
```

---

## 🏗️ System Architecture

### Class Hierarchy

```
Printable (Abstract)
    ├─ Entity
    │   ├─ Book        (title, author, copies, rating)
    │   ├─ User        (name, penalty status)
    │   └─ IssuedRecord (issue_id, user_id, dates)
    │
    └─ Library         (Main controller class)
```

### Database Schema

```sql
-- Books table
books (
  book_id INTEGER PRIMARY KEY,
  title TEXT,
  author TEXT,
  total_copies INTEGER,
  available_copies INTEGER,
  avg_rating REAL,
  total_ratings INTEGER
)

-- Users table
users (
  user_id INTEGER PRIMARY KEY,
  name TEXT,
  is_defaulter BOOLEAN,
  penalty_end TIMESTAMP
)

-- Current issues
issued (
  issue_id INTEGER PRIMARY KEY,
  book_id INTEGER FOREIGN KEY,
  user_id INTEGER FOREIGN KEY,
  issue_datetime TIMESTAMP,
  due_datetime TIMESTAMP
)

-- Historical records
history (
  issue_id INTEGER,
  book_id INTEGER,
  user_id INTEGER,
  title TEXT,
  author TEXT,
  issue_datetime TIMESTAMP,
  return_datetime TIMESTAMP,
  status TEXT
)
```

---

## 📁 Project Structure

```
library-management-system/
├── src/
│   └── lib_management_sys_sqlite3.cpp    (Main source, ~800 lines)
├── docs/
│   ├── INSTALL.md                        (Installation guide)
│   ├── USAGE.md                          (Detailed usage)
│   └── ARCHITECTURE.md                   (System design)
├── build/
│   └── lib_management.exe                (Compiled binary)
├── README.md                             (This file)
├── .gitignore                            (Git rules)
├── LICENSE                               (MIT License)
└── build.bat                             (Windows build script)
```

---

## 💡 Code Highlights

### Polymorphism Example
```cpp
// Abstract base class
struct Printable {
    virtual string info() const = 0;
};

// Derived classes override info()
struct Book : public Entity {
    string info() const override { ... }
};

struct User : public Entity {
    string info() const override { ... }
};
```

### Database Abstraction
```cpp
// Private SQL helpers (encapsulation)
class Library {
private:
    bool exec_sql(const char* sql);
    void load_books();
    void save_books();
    
public:
    // Public interface hides DB details
    void add_book(const Book& b);
    void view_books();
};
```

### CRUD Operations
```cpp
// Create
void add_book(const Book& b)

// Read
void view_books()

// Update
void update_book_availability()

// Delete
void delete_book(int id)
```

---

## 🔄 Workflow Example

### Issuing a Book (Admin Perspective)

```
1. View pending requests
   → Shows all user requests

2. Approve/Reject
   → Updates book availability
   → Creates issue record
   → Saves to database

3. Calculate due date
   → 14 days from issue

4. Log to history
   → Maintains audit trail
```

---

## 🛠️ Development & Contribution

### Build from Source

```bash
# Compile
g++ src/lib_management_sys_sqlite3.cpp -o lib_management -lsqlite3

# With debug symbols
g++ -g src/lib_management_sys_sqlite3.cpp -o lib_management -lsqlite3
```

### Code Standards

- **Naming**: camelCase for variables, snake_case for functions
- **Comments**: Explains "why", not "what"
- **Functions**: Single responsibility principle
- **Database**: Prepared statements (SQL injection prevention)

### Future Enhancements

- [ ] GUI using Qt/SFML
- [ ] Web interface (Flask/Django)
- [ ] Email notifications for due dates
- [ ] Advanced search and filtering
- [ ] User authentication with password hashing
- [ ] Book recommendation system
- [ ] Mobile app integration
- [ ] REST API

---

## 📊 Data Flow Diagram

```
User Input
    ↓
┌─────────────┐
│ Menu System │ (Admin/User Interface)
└──────┬──────┘
       ↓
┌──────────────────┐
│ Library Class    │ (Business Logic)
│ (Validation,     │
│  Processing)     │
└──────┬───────────┘
       ↓
┌──────────────────┐
│ SQLite3 DB       │ (Persistence)
│ (Tables, Queries)│
└──────────────────┘
```

---

## 🎓 Learning Outcomes

This project demonstrates:

✅ Object-Oriented Programming (OOP)
- Inheritance
- Polymorphism
- Encapsulation
- Abstraction

✅ Database Management
- SQL queries
- Schema design
- Foreign keys
- Data persistence

✅ Data Structures
- Hash maps (unordered_map)
- Structs and classes
- Timestamp handling

✅ File I/O & System Design
- Database file handling
- Memory management
- Error handling

---

## 🐛 Troubleshooting

### Issue: "Cannot open database"
```
Solution: Check disk space, permissions, or try deleting library.db
```

### Issue: "g++ not found"
```
Solution: Install MinGW-w64 and add to PATH (see INSTALL.md)
```

### Issue: "User cannot issue book"
```
Solution: User might already have 1 book issued (max = 1)
```

### Issue: Program crashes on entry
```
Solution: Recompile with: g++ -g ... (debug mode)
```

---

## 📝 License

This project is licensed under the **MIT License** - see [LICENSE](LICENSE) file for details.

**Summary**: You can use, modify, and distribute this code freely.

---

## 👤 Author

**Your Name** (@YourGitHubUsername)

- 🎓 BTech ECE, NSUT
- 💻 C++ | SQLite | System Design
- 📧 your.email@gmail.com
- 🔗 [GitHub Profile](https://github.com/YourUsername)

---

## ⭐ If This Helps You!

If this project was useful:
- Star ⭐ this repository
- Share with friends
- Leave feedback/suggestions
- Fork and improve it!

---

## 📚 Recommended Reading

- [C++ STL Reference](https://en.cppreference.com/)
- [SQLite Documentation](https://www.sqlite.org/docs.html)
- [Design Patterns](https://refactoring.guru/design-patterns)
- [System Design](https://github.com/donnemartin/system-design-primer)

---

**Happy Coding! 🚀**

Questions? Open an [Issue](https://github.com/YourUsername/library-management-system/issues)
