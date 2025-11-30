//g++ lib_management_sys_sqlite3.cpp -o lib_management_sys_sqlite3 -L. -l sqlite3
// ./y

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <limits>
#include <ctime>        // for time_t, localtime, time
#include "sqlite3.h"
#include <functional>

using namespace std;



// ----------------------
// Polymorphism / Inheritance base
// ----------------------
struct Printable {
    virtual string info() const = 0;
    virtual ~Printable() = default;
}; 

// ----------------------
// Entity: identity base (inherits Printable)
// ----------------------
class Entity : public Printable {
protected:
    int id;
public:
    Entity(int idarg = 0) : id(idarg) {

    }
    int getID() const { 
        return id; 
    }
    void setID(int v) {
         id = v; 
    }
    // Note: Printable::info remains pure virtual; concrete classes override it.
};

// ----------------------
// Book (inherits Entity)
// ----------------------
struct Book : public Entity {
    // no reference member anymore; use getID()
    string title;
    string author;
    int totalCopies;
    int availableCopies;
    double avg_rating;
    int total_ratings;

    Book() : Entity(0), title(""), author(""), totalCopies(0), availableCopies(0), avg_rating(0.0), total_ratings(0) {

    }
    Book(int id_, string t, string a, int tot, int avail, double rating = 0.0, int ratings = 0) 
        : Entity(id_), title(t), author(a), totalCopies(tot), availableCopies(avail), avg_rating(rating), total_ratings(ratings) {

    }

    // convenience accessor to mimic original .book_id usage
    int book_id() const {
         return getID();
    }

    string info() const override {
        std::ostringstream ss;
        ss << "ID: " << book_id() << " | Title: " << title 
           << " | Author: " << author 
           << " | Total: " << totalCopies 
           << " | Available: " << availableCopies
           << " | Rating: " << fixed << setprecision(1) << avg_rating;
        return ss.str();
    }
};

// ----------------------
// User (inherits Entity)
// ----------------------
struct User : public Entity {
    string name;
    bool isDefaulter;
    time_t penaltyEnd;

    User() : Entity(0), name(""), isDefaulter(false), penaltyEnd(0) {

    }
    User(int id_, string n) : Entity(id_), name(n), isDefaulter(false), penaltyEnd(0) {

    }

    int user_id() const { 
        return getID(); 
    }

    string info() const override {
        std::ostringstream ss;
        ss << "ID: " << user_id() << " | Name: " << name;
        if (isDefaulter && penaltyEnd > 0) {
            char buf[64] = {0};
            struct tm* tmv = localtime(&penaltyEnd);
            if (tmv) {
                strftime(buf, sizeof(buf), " | Defaulter until: %Y-%m-%d", tmv);
            }
            ss << buf;
        }
        return ss.str();
    }
};

// ----------------------
// IssuedRecord (inherits Entity)
// ----------------------
struct IssuedRecord : public Entity {
    int book_id;
    int user_id;
    time_t issueDatetime;
    time_t dueDatetime;

    IssuedRecord() : Entity(0), book_id(0), user_id(0), issueDatetime(0), dueDatetime(0) {

    }
    IssuedRecord(int iid, int bid, int uid, time_t issue, time_t due) 
        : Entity(iid), book_id(bid), user_id(uid), issueDatetime(issue), dueDatetime(due) {

    }

    int issue_id() const { 
        return getID(); 
    }

    string info() const override {
        std::ostringstream ss;
        ss << "Issued ID: " << issue_id() << " | Book ID: " << book_id 
           << " | User ID: " << user_id;
        return ss.str();
    }
};

// ----------------------
// Helper: print any Printable (demonstrates polymorphism)
// ----------------------
void printEntity(const Printable& p) {
    cout << p.info() << "\n";
}

// ----------------------
// Library class (encapsulation + abstraction)
// ----------------------
class Library {
private:
    sqlite3* db;
    unordered_map<int, Book> books;
    unordered_map<int, User> users;
    unordered_map<int, IssuedRecord> issued;  // key: issue_id

    const string DB_FILE = "library.db";
    const string ADMIN_PASS = "admin123";

    // SQLite helper functions (encapsulated)
    bool exec_sql(const char* sql) {
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            cout << "SQL error: " << (errMsg ? errMsg : "(unknown)") << endl;
            if (errMsg) sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    int get_last_insert_rowid() {
        return (int)sqlite3_last_insert_rowid(db);
    }

public:
    // Constructor opens DB, initializes schema and loads data
    Library() : db(nullptr) {
        if (sqlite3_open(DB_FILE.c_str(), &db) != SQLITE_OK) {
            cout << "Cannot open database" << endl;
            exit(1);
        }
        init_schema();
        load_all_data();
    }

    // Destructor saves and closes DB
    ~Library() {
        save_all();
        if (db) sqlite3_close(db);
    }

    // Initialize DB schema
    void init_schema() {
        const char* sql = R"(
            PRAGMA foreign_keys = ON;
            CREATE TABLE IF NOT EXISTS books (
                book_id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT,
                author TEXT,
                total_copies INTEGER,
                available_copies INTEGER,
                avg_rating REAL DEFAULT 0,
                total_ratings INTEGER DEFAULT 0
            );
            CREATE TABLE IF NOT EXISTS users (
                user_id INTEGER PRIMARY KEY,
                name TEXT,
                is_defaulter INTEGER DEFAULT 0,
                penalty_end INTEGER DEFAULT 0
            );
            CREATE TABLE IF NOT EXISTS issued (
                issue_id INTEGER PRIMARY KEY AUTOINCREMENT,
                book_id INTEGER,
                user_id INTEGER UNIQUE,
                issue_datetime INTEGER,
                due_datetime INTEGER,
                FOREIGN KEY (book_id) REFERENCES books(book_id),
                FOREIGN KEY (user_id) REFERENCES users(user_id)
            );
            CREATE TABLE IF NOT EXISTS history (
                issue_id INTEGER PRIMARY KEY,
                book_id INTEGER,
                user_id INTEGER,
                title TEXT,
                author TEXT,
                issue_datetime INTEGER,
                return_datetime INTEGER,
                status TEXT
            );
        )";
        exec_sql(sql);
    }

    // Load all data from DB to memory (abstraction hides DB details)
    void load_all_data() {
        load_books();
        load_users();
        load_issued();
    }

    void load_books() {
        books.clear();
        const char* sql = "SELECT book_id, title, author, total_copies, available_copies, avg_rating, total_ratings FROM books;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                string title = (const char*)sqlite3_column_text(stmt, 1);
                string author = (const char*)sqlite3_column_text(stmt, 2);
                int total = sqlite3_column_int(stmt, 3);
                int avail = sqlite3_column_int(stmt, 4);
                double rating = sqlite3_column_double(stmt, 5);
                int ratings = sqlite3_column_int(stmt, 6);
                books[id] = Book(id, title, author, total, avail, rating, ratings);
            }
            sqlite3_finalize(stmt);
        }
    }

    void load_users() {
        users.clear();
        const char* sql = "SELECT user_id, name, is_defaulter, penalty_end FROM users;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                string name = (const char*)sqlite3_column_text(stmt, 1);
                bool defaulter = sqlite3_column_int(stmt, 2) != 0;
                time_t penalty = (time_t)sqlite3_column_int64(stmt, 3);
                users[id] = User(id, name);
                users[id].isDefaulter = defaulter;
                users[id].penaltyEnd = penalty;
            }
            sqlite3_finalize(stmt);
        }
    }

    void load_issued() {
        issued.clear();
        const char* sql = "SELECT issue_id, book_id, user_id, issue_datetime, due_datetime FROM issued;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int iid = sqlite3_column_int(stmt, 0);
                int bid = sqlite3_column_int(stmt, 1);
                int uid = sqlite3_column_int(stmt, 2);
                time_t issue = (time_t)sqlite3_column_int64(stmt, 3);
                time_t due = (time_t)sqlite3_column_int64(stmt, 4);
                issued[iid] = IssuedRecord(iid, bid, uid, issue, due);
            }
            sqlite3_finalize(stmt);
        }
    }

    // Save everything to DB
    void save_all() {
        save_books();
        save_users();
        save_issued();
    }

    void save_books() {
        exec_sql("DELETE FROM books;");
        const char* sql = "INSERT INTO books (book_id, title, author, total_copies, available_copies, avg_rating, total_ratings) VALUES (?, ?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            for (auto& p : books) {
                const Book& b = p.second;
                sqlite3_bind_int(stmt, 1, b.book_id());  // use accessor
                sqlite3_bind_text(stmt, 2, b.title.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, b.author.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 4, b.totalCopies);
                sqlite3_bind_int(stmt, 5, b.availableCopies);
                sqlite3_bind_double(stmt, 6, b.avg_rating);
                sqlite3_bind_int(stmt, 7, b.total_ratings);
                sqlite3_step(stmt);
                sqlite3_reset(stmt);
            }
            sqlite3_finalize(stmt);
        }
    }

    void save_users() {
        exec_sql("DELETE FROM users;");
        const char* sql = "INSERT INTO users (user_id, name, is_defaulter, penalty_end) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            for (auto& p : users) {
                const User& u = p.second;
                sqlite3_bind_int(stmt, 1, u.user_id()); // accessor
                sqlite3_bind_text(stmt, 2, u.name.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 3, u.isDefaulter ? 1 : 0);
                sqlite3_bind_int64(stmt, 4, (sqlite3_int64)u.penaltyEnd);
                sqlite3_step(stmt);
                sqlite3_reset(stmt);
            }
            sqlite3_finalize(stmt);
        }
    }

    void save_issued() {
        exec_sql("DELETE FROM issued;");
        const char* sql = "INSERT INTO issued (issue_id, book_id, user_id, issue_datetime, due_datetime) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            for (auto& p : issued) {
                const IssuedRecord& r = p.second;
                sqlite3_bind_int(stmt, 1, r.issue_id()); // accessor
                sqlite3_bind_int(stmt, 2, r.book_id);
                sqlite3_bind_int(stmt, 3, r.user_id);
                sqlite3_bind_int64(stmt, 4, (sqlite3_int64)r.issueDatetime);
                sqlite3_bind_int64(stmt, 5, (sqlite3_int64)r.dueDatetime);
                sqlite3_step(stmt);
                sqlite3_reset(stmt);
            }
            sqlite3_finalize(stmt);
        }
    }

    // Helper functions
    void clearInputLine() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    int readInt(const string& prompt) {
        int x;
        cout << prompt;
        while (!(cin >> x)) {
            clearInputLine();
            cout << "Invalid input! Please enter a number: ";
        }
        return x;
    }

    int readMenuChoice() {
        int ch;
        cout << "Enter choice: ";
        while (!(cin >> ch)) {
            clearInputLine();
            cout << "Invalid choice! Please enter a number: ";
        }
        return ch;
    }

    string epochToStr(time_t t) {
        if (t == 0) return "-";
        char buf[64];
        struct tm* tmv = localtime(&t);
        strftime(buf, sizeof(buf), "%Y-%m-%d", tmv);
        return string(buf);
    }

    bool user_has_active_issue(int userId) const {
        for (auto& p : issued) {
            if (p.second.user_id == userId) return true;
        }
        return false;
    }

    // Book operations
    void addBook() {
        clearInputLine();
        string title, author;
        cout << "Enter Title: "; getline(cin, title);
        cout << "Enter Author: "; getline(cin, author);
        int total = readInt("Enter total copies: ");
        if (total <= 0) { cout << "Invalid number.\n"; return; }

        const char* sql = "INSERT INTO books (title, author, total_copies, available_copies) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, author.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 3, total);
            sqlite3_bind_int(stmt, 4, total);
            sqlite3_step(stmt);
            int book_id = get_last_insert_rowid();
            sqlite3_finalize(stmt);

            books[book_id] = Book(book_id, title, author, total, total);
            cout << "Book added successfully. ID: " << book_id << "\n";
        }
    }

    void removeBook() {
        int book_id = readInt("Enter Book ID to remove: ");
        if (books.find(book_id) == books.end()) {
            cout << "Book not found.\n";
            return;
        }

        // Check if any active issues
        for (auto& p : issued) {
            if (p.second.book_id == book_id) {
                cout << "Cannot remove; active issued copies exist.\n";
                return;
            }
        }

        char sql[256];
        sprintf(sql, "DELETE FROM books WHERE book_id = %d;", book_id);
        if (exec_sql(sql)) {
            books.erase(book_id);
            cout << "Book removed.\n";
        }
    }
    void viewBooks() {
    if (books.empty()) {
        cout << "No books available.\n";
        return;
    }

    cout << "\n------------------- BOOK LIST -------------------\n";
    cout << left << setw(6) << "ID"
         << setw(30) << "Title"
         << setw(20) << "Author"
         << setw(10) << "Total"
         << setw(12) << "Available"
         << setw(10) << "Rating"
         << "Ratings Count"
         << "\n";

    cout << string(90, '-') << "\n";

    for (auto& p : books) {
        const Book& b = p.second;

        cout << left
             << setw(6) << b.book_id()
             << setw(30) << b.title
             << setw(20) << b.author
             << setw(10) << b.totalCopies
             << setw(12) << b.availableCopies
             << setw(10) << fixed << setprecision(1) << b.avg_rating
             << b.total_ratings
             << "\n";
    }
}
// User operations
    void addUser() {
        int id = readInt("Enter User ID: ");
        if (users.count(id)) { cout << "User exists.\n"; return; }

        clearInputLine();
        string name; cout << "Enter Name: "; getline(cin, name);

        users[id] = User(id, name);
        char sql[512];
        sprintf(sql, "INSERT INTO users (user_id, name) VALUES (%d, '%s');", id, name.c_str());
        exec_sql(sql);
        cout << "User added.\n";
    }
    void removeUser() {
        int id = readInt("Enter User ID to remove: ");
        if (!users.count(id)) { cout << "User not found.\n"; return; }

        if (user_has_active_issue(id)) {
            cout << "Cannot remove; user has active issued book.\n";
            return;
        }

        char sql[256];
        sprintf(sql, "DELETE FROM users WHERE user_id = %d;", id);
        if (exec_sql(sql)) {
            users.erase(id);
            cout << "User removed.\n";
        }
    }
    void viewUsers() {
    if (users.empty()) {
        cout << "No users.\n";
        return;
    }

    time_t now = time(0);

    cout << "\n-------------------------------------------------------------------------------------------\n";
    cout << left << setw(8)  << "ID"
         << setw(20) << "Name"
         << setw(12) << "Status"
         << setw(10) << "BookID"
         << setw(15) << "Issue Date"
         << setw(15) << "Due Date"
         << setw(15) << "Penalty End"
         << "\n-------------------------------------------------------------------------------------------\n";

    for (auto &p : users) {
        const User &u = p.second;

        string status = "ACTIVE";
        int issuedBookId = -1;
        time_t issueTime = 0, dueTime = 0;
        string issueStr = "-", dueStr = "-", penaltyStr = "-";

        // Check defaulter
        if (u.isDefaulter && now < u.penaltyEnd) {
            status = "DEFAULTER";
            penaltyStr = epochToStr(u.penaltyEnd);
        }

        // Check issued
        for (auto &q : issued) {
            if (q.second.user_id == u.user_id()) {
                status = "ISSUED";
                issuedBookId = q.second.book_id;
                issueTime = q.second.issueDatetime;
                dueTime = q.second.dueDatetime;
                issueStr = epochToStr(issueTime);
                dueStr = epochToStr(dueTime);
                break;
            }
        }

        cout << left << setw(8)  << u.user_id()
             << setw(20) << u.name
             << setw(12) << status
             << setw(10) << (issuedBookId == -1 ? "-" : to_string(issuedBookId))
             << setw(15) << issueStr
             << setw(15) << dueStr
             << setw(15) << penaltyStr
             << "\n";
    }

    cout << "-------------------------------------------------------------------------------------------\n";
}
// Issue/Return operations
    void user_request_issue() {
        int uid = readInt("Enter your User ID: ");
        if (!users.count(uid)) {
            cout << "User not found. Register? (1=Yes 2=No): ";
            int ch = readMenuChoice();
            if (ch == 1) {
                clearInputLine();
                string name; cout << "Enter Name: "; getline(cin, name);
                users[uid] = User(uid, name);
                char sql[512];
                sprintf(sql, "INSERT INTO users (user_id, name) VALUES (%d, '%s');", uid, name.c_str());
                exec_sql(sql);
                cout << "Registered successfully.\n";
            } else {
                cout << "Operation cancelled.\n";
                return;
            }
        }

        User& u = users[uid];
        time_t now = time(0);
        if (u.isDefaulter && now < u.penaltyEnd) {
            cout << "You are a defaulter until: " << epochToStr(u.penaltyEnd) << "\n";
            return;
        }

        if (user_has_active_issue(uid)) {
            cout << "You already have an active issued book.\n";
            return;
        }

        viewBooks();
        int book_id = readInt("Enter Book ID to issue: ");
        if (books.find(book_id) == books.end()) {
            cout << "Book not found.\n";
            return;
        }

        Book& b = books[book_id];
        if (b.availableCopies <= 0) {
            cout << "No available copies.\n";
            return;
        }

        // Issue book
        b.availableCopies--;
        time_t issueTime = now;
        time_t dueTime = issueTime + (15LL * 24 * 60 * 60); // 15 days

        const char* sql_issue = "INSERT INTO issued (book_id, user_id, issue_datetime, due_datetime) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql_issue, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, book_id);
            sqlite3_bind_int(stmt, 2, uid);
            sqlite3_bind_int64(stmt, 3, (sqlite3_int64)issueTime);
            sqlite3_bind_int64(stmt, 4, (sqlite3_int64)dueTime);
            sqlite3_step(stmt);
            int issue_id = get_last_insert_rowid();
            sqlite3_finalize(stmt);

            issued[issue_id] = IssuedRecord(issue_id, book_id, uid, issueTime, dueTime);

            // Add to history
            const char* sql_history = "INSERT INTO history (issue_id, book_id, user_id, title, author, issue_datetime, return_datetime, status) VALUES (?, ?, ?, ?, ?, ?, 0, 'issued');";
            if (sqlite3_prepare_v2(db, sql_history, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, issue_id);
                sqlite3_bind_int(stmt, 2, book_id);
                sqlite3_bind_int(stmt, 3, uid);
                sqlite3_bind_text(stmt, 4, b.title.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 5, b.author.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int64(stmt, 6, (sqlite3_int64)issueTime);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }

            // Update book
            char sql_update[256];
            sprintf(sql_update, "UPDATE books SET available_copies = %d WHERE book_id = %d;", b.availableCopies, book_id);
            exec_sql(sql_update);

            cout << "Issued successfully! Issue ID: " << issue_id << " | Due: " << epochToStr(dueTime) << "\n";
        }
    }
    void user_request_return() {
    int uid = readInt("Enter your User ID: ");
    if (!users.count(uid)) { cout << "User not found.\n"; return; }

    int issue_id = -1;
    for (auto& p : issued) {
        if (p.second.user_id == uid) {
            issue_id = p.first;
            break;
        }
    }

    if (issue_id == -1) {
        cout << "No active issued books.\n";
        return;
    }

    IssuedRecord& rec = issued[issue_id];
    time_t now = time(0);

    // Update book availability
    if (books.count(rec.book_id)) {
        Book& b = books[rec.book_id];
        b.availableCopies++;
        if (b.availableCopies > b.totalCopies) b.availableCopies = b.totalCopies;

        char sql_update[256];
        sprintf(sql_update, "UPDATE books SET available_copies = %d WHERE book_id = %d;", b.availableCopies, rec.book_id);
        exec_sql(sql_update);

        // --------------------------
        // ⭐ ASK FOR RATING 1–5
        // --------------------------
        int rating;
        cout << "Rate the book (1 to 5 stars): ";
        cin >> rating;

        while (rating < 1 || rating > 5) {
            cout << "Invalid rating! Enter a number between 1 and 5: ";
            cin >> rating;
        }

        // ⭐ Update rating logic
        b.total_ratings++;
        b.avg_rating = ((b.avg_rating * (b.total_ratings - 1)) + rating) / b.total_ratings;

        // Save updated rating to DB
        char sql_rating[256];
        sprintf(sql_rating,
                "UPDATE books SET avg_rating = %.2f, total_ratings = %d WHERE book_id = %d;",
                b.avg_rating, b.total_ratings, rec.book_id);
        exec_sql(sql_rating);
    }

    // Remove from issued
    char sql_delete[256];
    sprintf(sql_delete, "DELETE FROM issued WHERE issue_id = %d;", issue_id);
    exec_sql(sql_delete);
    issued.erase(issue_id);

    // Update history
    string status = "returned";
    if (now > rec.dueDatetime) {
        status = "defaulter";
        users[uid].isDefaulter = true;
        users[uid].penaltyEnd = now + (7LL * 24 * 60 * 60); // 7 days penalty
        char sql_user[512];
        sprintf(sql_user, "UPDATE users SET is_defaulter = 1, penalty_end = %ld WHERE user_id = %d;",
                (long)users[uid].penaltyEnd, uid);
        exec_sql(sql_user);
        cout << "Overdue return! You are marked as defaulter. Penalty until: "
             << epochToStr(users[uid].penaltyEnd) << "\n";
    } else {
        cout << "Book returned successfully. Thank you!\n";
    }

    char sql_history[512];
    sprintf(sql_history,
            "UPDATE history SET return_datetime = %ld, status = '%s' WHERE issue_id = %d;",
            (long)now, status.c_str(), issue_id);
    exec_sql(sql_history);
}
    void user_check_status() {
        int uid = readInt("Enter your User ID: ");
        if (!users.count(uid)) { cout << "User not found.\n"; return; }

        User& u = users[uid];
        time_t now = time(0);
        bool active = !user_has_active_issue(uid) && !(u.isDefaulter && now < u.penaltyEnd);
        cout << "User " << uid << " (" << u.name << ") is " << (active ? "ACTIVE" : "DISABLED") << ".\n";

        for (auto& p : issued) {
            if (p.second.user_id == uid) {
                cout << "Issued ID: " << p.first << " | Issued: " << epochToStr(p.second.issueDatetime) 
                     << " | Due: " << epochToStr(p.second.dueDatetime) << "\n";
            }
        }

        if (u.isDefaulter && now < u.penaltyEnd) {
            cout << "Penalty until: " << epochToStr(u.penaltyEnd) << "\n";
        }
    }

    // Admin menu functions
    void listDefaulters() {
        time_t now = time(0);
        bool any = false;
        for (auto& p : users) {
            const User& u = p.second;
            if (u.isDefaulter && now < u.penaltyEnd) {
                any = true;
                cout << "ID: " << u.user_id() << " | " << u.name << " | Penalty ends: " << epochToStr(u.penaltyEnd) << "\n";
                for (auto& q : issued) {
                    if (q.second.user_id == u.user_id()) {
                        cout << "  Active: ID " << q.first << " | Due: " << epochToStr(q.second.dueDatetime) << "\n";
                    }
                }
            }
        }
        if (!any) cout << "No defaulters.\n";
    }

    void viewHistoryLastN(int N) {
        if (N <= 0) return;
        char sql[256];
        sprintf(sql, "SELECT * FROM history ORDER BY issue_id DESC LIMIT %d;", N);

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                int issue_id = sqlite3_column_int(stmt, 0);
                int book_id = sqlite3_column_int(stmt, 1);
                int user_id = sqlite3_column_int(stmt, 2);
                string title = (const char*)sqlite3_column_text(stmt, 3);
                string author = (const char*)sqlite3_column_text(stmt, 4);
                time_t issue = (time_t)sqlite3_column_int64(stmt, 5);
                time_t ret = (time_t)sqlite3_column_int64(stmt, 6);
                string status = (const char*)sqlite3_column_text(stmt, 7);

                cout << "ID: " << issue_id << " | Title: " << title << " | Author: " << author 
                     << " | User: " << user_id << " | Issued: " << epochToStr(issue)
                     << " | Returned: " << (ret == 0 ? "-" : epochToStr(ret))
                     << " | Status: " << status << "\n";
            }
            sqlite3_finalize(stmt);
        }
    }

    // Menus
    void admin_menu() {
        cout << "Enter admin password: ";
        string pass; cin >> pass;
        if (pass != ADMIN_PASS) { cout << "Wrong password.\n"; return; }

        int choice;
        while (true) {
            cout << "\n--- ADMIN MENU ---\n";
            cout << "1. Add Book\n2. Remove Book\n3. View Books\n4. Add User\n5. Remove User\n6. View Users\n";
            cout << "7. List Defaulters\n8. View History (last N)\n9. Save All\n0. Exit\n";
            choice = readMenuChoice();

            switch (choice) {
                case 1: addBook(); break;
                case 2: removeBook(); break;
                case 3: viewBooks(); break;
                case 4: addUser(); break;
                case 5: removeUser(); break;
                case 6: viewUsers(); break;
                case 7: listDefaulters(); break;
                case 8: {
                    int N = readInt("Enter number of recent records: ");
                    viewHistoryLastN(N);
                    break;
                }
                case 9: save_all(); cout << "Saved all.\n"; break;
                case 0: return;
                default: cout << "Invalid choice.\n";
            }
        }
    }

    void user_menu() {
        int choice;
        while (true) {
            cout << "\n--- USER MENU ---\n";
            cout << "1. View Books\n2. Issue Book\n3. Return Book\n4. Check Status\n0. Exit\n";
            choice = readMenuChoice();

            switch (choice) {
                case 1: viewBooks(); break;
                case 2: user_request_issue(); break;
                case 3: user_request_return(); break;
                case 4: user_check_status(); break;
                case 0: return;
                default: cout << "Invalid choice.\n";
            }
        }
    }
};

int main() {
    Library lib;
    

    int choice;

    while (true) {
        cout << "\n===== Library Management System =====\n";
        cout << "1. Admin\n2. User\n3. Exit\n";
        choice = lib.readMenuChoice();

        switch (choice) {
            case 1: lib.admin_menu(); break;
            case 2: lib.user_menu(); break;
            case 3: cout << "Goodbye!\n"; return 0;
            default: cout << "Invalid choice.\n";
        }
    }

    return 0;
}
