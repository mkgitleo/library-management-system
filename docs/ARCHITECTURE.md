# System Architecture & Design Documentation

## Table of Contents

1. [Overview](#overview)
2. [Design Principles](#design-principles)
3. [Class Architecture](#class-architecture)
4. [Database Design](#database-design)
5. [Key Algorithms](#key-algorithms)
6. [Design Patterns Used](#design-patterns-used)

---

## Overview

The Library Management System is a C++ application that demonstrates professional software architecture principles. It uses SQLite3 for persistent data storage and implements a clean, object-oriented design.

### System Goals

✅ Manage library books and user records
✅ Track book issuance and returns
✅ Enforce penalties for overdue books
✅ Provide role-based access (Admin/User)
✅ Maintain complete audit trail
✅ Ensure data persistence

---

## Design Principles

### 1. Separation of Concerns

```
Presentation Layer (Menu System)
        ↓
Business Logic Layer (Library Class)
        ↓
Data Access Layer (SQLite3 Queries)
        ↓
Database Layer (SQL Tables)
```

**Benefits**:
- Easy to maintain
- Easy to test
- Easy to extend

### 2. Object-Oriented Principles

#### Encapsulation
- Private data members
- Public interface methods
- Controlled access

#### Inheritance
- `Printable` base class (abstract)
- `Entity` base class (abstract)
- `Book`, `User`, `IssuedRecord` derived classes

#### Polymorphism
- Virtual functions: `info()`, `print()`
- Runtime type determination
- Different behaviors for different entities

#### Abstraction
- Hide implementation details
- Expose only necessary interface
- Use abstract base classes

### 3. Single Responsibility Principle (SRP)

Each class has one job:
- `Book` - Represents a book
- `User` - Represents a user
- `IssuedRecord` - Represents a transaction
- `Library` - Manages the system

### 4. DRY (Don't Repeat Yourself)

- Common functionality in base classes
- Reusable helper functions
- Consistent error handling

---

## Class Architecture

### Class Hierarchy

```
┌─────────────────┐
│   Printable     │ (Abstract - defines info() interface)
└────────┬────────┘
         │
         ├─────────────────────────────────┐
         │                                 │
    ┌────▼──────────┐              ┌──────▼──────────┐
    │    Entity     │              │   Library       │
    │  (Abstract)   │              │  (Controller)   │
    └────┬──────────┘              └─────────────────┘
         │
    ┌────┴────────────────────┐
    │                         │
┌───▼────┐         ┌──────────▼────┐        ┌────────────────┐
│  Book  │         │     User      │        │  IssuedRecord  │
└────────┘         └───────────────┘        └────────────────┘
```

### Detailed Class Structure

#### Printable (Abstract)
```cpp
struct Printable {
    virtual string info() const = 0;  // Pure virtual
    virtual ~Printable() {}
};
```

**Purpose**: Define printing interface for all entities

#### Entity (Abstract)
```cpp
struct Entity : Printable {
    int id;
    virtual string info() const = 0;
};
```

**Purpose**: Common base for all domain entities

#### Book
```cpp
struct Book : Entity {
    string title;
    string author;
    int total_copies;
    int available_copies;
    double avg_rating;
    int total_ratings;
    
    string info() const override { /* format book info */ }
};
```

**Responsibilities**:
- Store book data
- Calculate availability
- Track ratings

#### User
```cpp
struct User : Entity {
    string name;
    bool is_defaulter;
    time_t penalty_end;
    
    string info() const override { /* format user info */ }
};
```

**Responsibilities**:
- Store user data
- Track penalty status
- Manage borrowing capacity

#### IssuedRecord
```cpp
struct IssuedRecord : Entity {
    int book_id;
    int user_id;
    time_t issue_datetime;
    time_t due_datetime;
    
    string info() const override { /* format record info */ }
};
```

**Responsibilities**:
- Record book issuance
- Track due dates
- Manage returns

#### Library (Main Controller)
```cpp
class Library {
private:
    // Encapsulated state
    vector<Book> books;
    vector<User> users;
    vector<IssuedRecord> issued;
    unordered_map<int, Book> book_map;
    unordered_map<int, User> user_map;
    sqlite3* db;
    
    // Helper functions
    bool exec_sql(const char* sql);
    void load_books();
    void save_books();
    
public:
    // Public interface
    void admin_menu();
    void user_menu();
    void add_book();
    // ... other operations
};
```

**Responsibilities**:
- Orchestrate system operations
- Manage database connections
- Handle business logic
- Enforce rules (penalties, limits, etc.)

---

## Database Design

### Schema

#### books Table
```sql
CREATE TABLE books (
    book_id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    author TEXT NOT NULL,
    total_copies INTEGER CHECK(total_copies > 0),
    available_copies INTEGER CHECK(available_copies >= 0),
    avg_rating REAL CHECK(avg_rating >= 0 AND avg_rating <= 5),
    total_ratings INTEGER CHECK(total_ratings >= 0)
);
```

**Indexes**: Likely on `title`, `author` for faster searches

#### users Table
```sql
CREATE TABLE users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    is_defaulter BOOLEAN DEFAULT 0,
    penalty_end DATETIME
);
```

**Indexes**: Likely on `name` for faster lookups

#### issued Table
```sql
CREATE TABLE issued (
    issue_id INTEGER PRIMARY KEY AUTOINCREMENT,
    book_id INTEGER NOT NULL FOREIGN KEY REFERENCES books,
    user_id INTEGER NOT NULL FOREIGN KEY REFERENCES users,
    issue_datetime DATETIME DEFAULT CURRENT_TIMESTAMP,
    due_datetime DATETIME NOT NULL
);
```

**Indexes**: On `user_id` (for finding user's books), `book_id` (for availability)

#### history Table
```sql
CREATE TABLE history (
    issue_id INTEGER,
    book_id INTEGER,
    user_id INTEGER,
    title TEXT,
    author TEXT,
    issue_datetime DATETIME,
    return_datetime DATETIME,
    status TEXT  -- 'returned', 'overdue', etc.
);
```

**Purpose**: Audit trail of all transactions

### Relationships

```
books (1) ────────── (many) issued ────────── (many) users
    │                            │
    └────────────────────────────┘
                 │
            history (audit log)
```

---

## Key Algorithms

### 1. Book Issue Algorithm

```
Input: user_id, book_id
Process:
    1. Check if user exists
    2. Check if user is defaulter
    3. Check if user already has book
    4. Check if copies available
    5. Create issued record
    6. Decrement available_copies
    7. Log to history
Output: Success/Failure with reason
```

### 2. Penalty Calculation

```
if (today > due_date) {
    days_late = today - due_date
    penalty = days_late * penalty_per_day
    penalty_end = today + (penalty_duration)
    mark_as_defaulter(user_id)
}
```

### 3. Search Algorithm

```
Search method: Linear search through vector
    → Could be optimized with hash map (already used)
    
Alternative: Could use binary search if sorted
```

---

## Design Patterns Used

### 1. Template Method Pattern (Implicit)

```cpp
// Base class defines structure
struct Printable {
    virtual string info() const = 0;
};

// Derived classes implement details
struct Book : Printable {
    string info() const override { /* Book-specific format */ }
};
```

### 2. Facade Pattern

```cpp
// Library class provides simple interface
// to complex subsystem (database, business logic)
public:
    void add_book();           // Complex: validation + DB + update
    void issue_book();         // Complex: check + update + log
```

### 3. Strategy Pattern (Could be improved)

```cpp
// Different access strategies for admin/user
if (user_role == ADMIN) {
    admin_menu();  // Different behavior
} else {
    user_menu();   // Different behavior
}
```

### 4. Repository Pattern

```cpp
// Library manages data persistence
private:
    bool exec_sql(const char* sql);
    void load_books();
    void save_books();
    
// Abstracts database details from business logic
```

---

## Data Flow Diagrams

### Book Issue Flow

```
User selects "Issue Book"
    ↓
Input: user_id, book_id
    ↓
Library::issue_book()
    ↓
Validation:
    - User exists? ✓
    - Not defaulter? ✓
    - Book exists? ✓
    - Copies available? ✓
    ↓
Create IssuedRecord
    ↓
Update books table:
    available_copies--
    ↓
Insert into issued table
    ↓
Log to history table
    ↓
Success message
```

### Penalty Check Flow

```
Return book
    ↓
Check: today > due_date?
    ↓
    YES → Apply penalty
            - Mark defaulter
            - Set penalty_end date
            - Reduce max books they can issue
    ↓
    NO → Normal return
            - Increment available_copies
            ↓
Log to history
    ↓
Complete
```

---

## Performance Considerations

### Time Complexity

| Operation | Complexity | Note |
|-----------|-----------|------|
| Add book | O(n) | Linear - vector insertion |
| Find book | O(1) avg | Using hash map |
| Issue book | O(1) avg | Hash map lookup + DB query |
| List books | O(n) | Linear - vector iteration |

### Space Complexity

- **Books vector**: O(n) where n = number of books
- **Users vector**: O(m) where m = number of users
- **Hash maps**: O(k) for k entities
- **Database**: O(t) where t = total transactions

### Optimization Opportunities

1. **Indexing**: Add database indexes on frequently searched fields
2. **Caching**: Cache loaded data to reduce DB queries
3. **Lazy Loading**: Load data on demand
4. **Connection Pooling**: Reuse database connections

---

## Error Handling Strategy

### Input Validation
```cpp
// Check user input at each step
if (book_id <= 0) {
    cout << "Invalid book ID\n";
    return;
}
```

### Database Errors
```cpp
// Handle SQLite3 errors
if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
    cerr << "SQL error: " << err_msg << "\n";
    sqlite3_free(err_msg);
}
```

### Business Logic Errors
```cpp
// Enforce business rules
if (is_defaulter) {
    cout << "Cannot issue book - you are a defaulter\n";
    return;
}
```

---

## Extension Points

### How to Extend

1. **Add new entity type**: Create new class inheriting from `Entity`
2. **Add new database table**: Create table, add CRUD operations
3. **Add new business rule**: Modify relevant method in `Library`
4. **Add new role**: Extend admin/user distinction
5. **Add new report**: Create query and display logic

### Future Enhancements

- GUI using Qt/SFML
- Web interface
- Email notifications
- Mobile app
- Advanced analytics
- Book recommendations

---

## Security Considerations

### Current Implementation
- Simple password for admin
- Role-based access control
- Input validation

### Potential Improvements
- Password hashing (bcrypt, argon2)
- SQL injection prevention (already using parameterized queries conceptually)
- User authentication with tokens
- Audit logging with timestamps
- Access control lists (ACL)

---

## Testing Strategy

### Unit Tests (Recommended)
- Test Book class validation
- Test User penalty logic
- Test date calculations

### Integration Tests
- Test book issue workflow
- Test penalty application
- Test database persistence

### System Tests
- Test admin workflows
- Test user workflows
- Test data integrity

---

## Maintenance Notes

### Code Quality
- ~800 lines, well-organized
- Clear class structure
- Documented algorithms
- Consistent naming

### Common Issues
- SQLite3 version compatibility
- Platform-specific path handling
- Memory management in C++

### Future Refactoring
- Extract database access to separate class
- Implement proper logging framework
- Add unit testing framework
- Consider using design patterns library

---

**This architecture is scalable and maintainable!** ✅
