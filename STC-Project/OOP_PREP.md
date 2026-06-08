# OOP Preparation Guide for STC Project

## 1. What this project uses

This project is a C++ CGI backend with object-oriented design. The key OOP concepts you must know are:

- **Class**: A blueprint for objects. Each class combines data and methods.
- **Object**: An instance of a class, like a customer or wallet.
- **Encapsulation**: Keeping data private and using public methods to access or change it.
- **Inheritance**: One class extends another and reuses its data and behavior.
- **Abstraction**: Hiding implementation details behind a defined interface.
- **Polymorphism**: Using a base class pointer to call derived class functions at runtime.
- **Composition**: A class contains another class as a member.
- **Constructor / Destructor**: Setup and cleanup logic for objects.

## 2. Important classes and how they work

### `Wallet`
- Stores `balance` and `transactionHistory` privately.
- Uses methods like `addMoney()` and `deductMoney()` to update balance safely.
- Provides `getBalance()` and `getTransactionHistory()` to read data.
- `toJSON()` converts wallet state to a JSON string.

Why it matters:
- This is a core example of encapsulation.
- The wallet object controls its own money operations.

### `User`
- Base class with shared fields: `id`, `username`, `password`, `email`, `role`.
- Contains getters and setters.
- Has a pure virtual method: `virtual string getRole() const = 0;`.
- This makes `User` an abstract class.

Why it matters:
- You cannot create a plain `User` object.
- It defines a shared interface for all user types.

### `Customer` and `Admin`
- Both inherit from `User`.
- `Customer` adds `phone`, `address`, and a `Wallet` object.
- `Admin` is a simpler user type with no extra fields.
- Both override `getRole()` and `toJSON()`.

Why it matters:
- This is inheritance + polymorphism.
- A `User*` pointer can point to either a `Customer` or `Admin`.

### `MenuItem`
- Represents a menu item with `id`, `name`, `description`, `price`, etc.
- Has getters, setters, and `toJSON()`.

Why it matters:
- This is a model object used for menu data.

### `Cart`
- Tracks items per customer using `map<int, int>`.
- Has methods to add, remove, clear items.
- Converts cart contents to JSON.

Why it matters:
- Demonstrates object behavior on stored data.

### `Order`
- Stores order details and status.
- Has `getProgressPercentage()` to convert status into a progress value.
- Serializes orders with `toJSON()`.

Why it matters:
- Helps understand order flow and status tracking.

### `Notification`, `Payment`, `Review`, `Announcement`, `Coupon`, `Banner`
- Each class stores its own related data.
- `Payment` also contains behavior: `processPayment(Wallet& customerWallet)`.

Why it matters:
- These examples show how different entities are modeled.
- `Payment` shows class behavior that works with another object.

## 3. Where OOP appears in code

### In `backend/classes.h`
- All class definitions appear here.
- This file is the main object model for your backend.

### In `backend/database.h`
- `DatabaseManager` is a class that handles file or MySQL storage.
- It has helper methods and returns objects like `User*`, `vector<MenuItem>`, and `Order`.

### In `backend/backend.cpp`
- The main CGI program creates `DatabaseManager db;`.
- It reads request action and returns JSON from objects.
- Example: `User* user = db.loginUser(username, password);` and `user->toJSON()`.

## 4. Live code change preparation

If your teacher asks you to change code live, use this checklist:

1. Identify the target class.
   - Example: change wallet logic in `Wallet` or `Payment`.
2. Update the class fields and constructor if you add a new property.
3. Add or update getters/setters.
4. Update `toJSON()` so new fields appear in output.
5. Update database methods if the class is saved/loaded from files.
6. Update route handlers in `backend/backend.cpp` if the change affects incoming actions.

## 5. Example “line-by-line” understanding style

When reading a class, explain lines like this:

- `class Wallet {` — defines a new class named `Wallet`.
- `private:` — marks the following variables as hidden from outside code.
- `double balance;` — stores how much money the customer has.
- `vector<string> transactionHistory;` — keeps a list of wallet actions.
- `Wallet() { ... }` — constructor that sets starting balance.
- `bool deductMoney(double amount, const string& description = "Payment Deducted") { ... }` — method to subtract money if enough balance exists.
- `string toJSON() const { ... }` — method that produces a JSON string for this wallet.

## 6. Common teacher questions and answers

- “What is inheritance here?”
  - `Customer` and `Admin` inherit from `User`.
- “What is polymorphism here?”
  - `User* user = db.loginUser(...)` can hold either a `Customer` or `Admin`, and `user->toJSON()` calls the correct version.
- “What is encapsulation here?”
  - Private fields like `balance` and public methods like `addMoney()` keep data safe.
- “What is composition here?”
  - `Customer` contains a `Wallet` object.

## 7. Quick tips for answering questions

- Mention the file names: `classes.h`, `database.h`, `backend.cpp`, `cgi_helper.h`.
- Explain that `classes.h` defines data models, `database.h` handles storage, and `backend.cpp` handles requests.
- Use the terms “object”, “class”, “inheritance”, “override”, and “encapsulation”.
- If asked about flow, say: user request → backend route → `DatabaseManager` → model objects → JSON response.

## 8. Use this file to practice

- Read a section and then open that class in `classes.h`.
- Try to explain what each method does out loud.
- If asked to change something, point to the exact class and method location.
- Practice adding a small property like `string city` to `Customer` and updating `toJSON()`.
