# OOP Concepts Used in the STC Project

This file explains each object-oriented programming concept used in this project and shows where it appears in your C++ backend code.

## 1. Class

A class is a blueprint for creating objects. It defines data and the functions that operate on that data.

Examples in this project:
- `Wallet`
- `User`
- `Customer`
- `Admin`
- `MenuItem`
- `Cart`
- `Order`
- `Notification`
- `Payment`
- `Review`
- `Announcement`
- `Coupon`
- `Banner`
- `DatabaseManager`

In `backend/classes.h`, each of these is defined as a `class` with fields and methods.


## 2. Object

An object is an instance of a class.

Example:
- `Wallet wallet;` inside `Customer`
- `DatabaseManager db;` in `backend/backend.cpp`
- `vector<MenuItem> menu = db.getMenu();`

Objects store values and allow method calls such as `wallet.addMoney(100)`.


## 3. Encapsulation

Encapsulation means keeping the internal state of an object private and exposing only the methods that are safe to use.

In this project:
- `Wallet` stores `balance` and `transactionHistory` as `private`.
- `User` stores `id`, `username`, `password`, `email`, and `role` as `protected`.
- `MenuItem`, `Order`, `Notification`, `Payment`, and others store fields as `private`.

Public methods provide controlled access:
- `Wallet::getBalance()` returns the balance.
- `Wallet::deductMoney()` changes balance only if the amount is valid.
- `Customer::getPhone()` and `Customer::setPhone()` read and modify phone safely.

Encapsulation protects data and prevents outside code from changing object internals directly.


## 4. Access Specifiers: private / protected / public

Access specifiers control who can see class members.

- `private`: only code inside the class can access the member.
- `protected`: the class itself and derived child classes can access the member.
- `public`: any code can access the member.

Examples:
- `Wallet` uses `private` for `balance` and `transactionHistory`.
- `User` uses `protected` for `id`, `username`, `password`, `email`, and `role`, so `Customer` and `Admin` can reuse them.
- `Customer` and `Admin` methods are `public` so the rest of the program can call them.


## 5. Constructor

A constructor is a special method called when an object is created. It initializes the object.

Examples:
- `Wallet::Wallet()` gives a starting balance of 100 and logs a welcome deposit.
- `Wallet::Wallet(double initialBalance)` initializes the wallet with a custom balance.
- `User::User(int uId, const string& uName, const string& uPass, const string& uEmail, const string& uRole)` initializes user fields.
- `Customer::Customer(int uId, const string& uName, const string& uPass, const string& uEmail, const string& uPhone, const string& uAddress, double walletBal = 100.0)` initializes the base `User` and the `Wallet`.

Constructors are used everywhere in your code to create objects with clean initial state.


## 6. Destructor

A destructor is a special method called when an object is destroyed. It can free resources.

Examples:
- `Wallet::~Wallet()` is defined but empty.
- `User::~User()` is virtual so deleting a derived object through a `User*` pointer calls the correct destructor.

In this project the destructors are simple, but the virtual destructor in `User` is important for safe polymorphism.


## 7. Inheritance

Inheritance lets one class reuse data and behavior from another class.

In this project:
- `Customer` inherits from `User` using `class Customer : public User`.
- `Admin` inherits from `User` using `class Admin : public User`.

That means `Customer` and `Admin` automatically have `id`, `username`, `password`, `email`, and `role` from `User`.

Example:
- `Customer` does not need to redefine `getUsername()` or `setEmail()` because it inherits them from `User`.


## 8. Abstraction

Abstraction means hiding implementation details and exposing only the important features.

In this project, `User` is an abstract base class because it contains a pure virtual function:
- `virtual string getRole() const = 0;`

This means `User` describes what a user can do, but does not define the exact type.

Concrete derived classes are:
- `Customer`
- `Admin`

Each of them provides a real implementation of `getRole()`.


## 9. Pure virtual function

A pure virtual function forces child classes to implement it.

Example:
- `User::getRole()` is pure virtual.
- `Customer::getRole()` returns "Customer".
- `Admin::getRole()` returns "Admin".

This makes `User` abstract and ensures every user type has a role description.


## 10. Polymorphism

Polymorphism means the same code can work with different types of objects.

In this project:
- `DatabaseManager::loginUser(username, password)` returns a pointer of type `User*`.
- At runtime, that pointer can point to a `Customer` or an `Admin` object.
- Calling `user->toJSON()` executes the derived class version of `toJSON()` because it is a virtual function.

Example from `backend.cpp`:
```cpp
User* user = db.loginUser(username, password);
if (user != NULL) {
    response << "{\"status\":\"success\",\"user\":" << user->toJSON() << "}";
    delete user;
}
```

Because of polymorphism, this code works for both customer login and admin login.


## 11. Overriding

Overriding means a derived class provides its own version of a virtual method from the base class.

Examples:
- `Customer::toJSON()` overrides `User::toJSON()` to include `phone`, `address`, and `wallet`.
- `Admin::toJSON()` overrides `User::toJSON()` to include admin-specific fields.
- `Customer::getRole()` and `Admin::getRole()` override `User::getRole()`.


## 12. Composition

Composition means one class contains another class as a member.

Example:
- `Customer` has a `Wallet wallet;`
- `Payment::processPayment(Wallet& customerWallet)` uses a `Wallet` object.

This is often described as a "has-a" relationship:
- A customer has a wallet.
- A review belongs to a menu item.


## 13. Method overloading

Method overloading means the same function name is used with different parameters.

Examples in `Wallet`:
- `Wallet()`
- `Wallet(double initialBalance)`

These are two constructors with the same name but different parameter lists.


## 14. Serialization method

Serialization means converting an object into a string format, usually JSON.

Examples:
- `Wallet::toJSON()`
- `Customer::toJSON()`
- `Admin::toJSON()`
- `MenuItem::toJSON()`
- `Order::toJSON()`
- `Notification::toJSON()`
- `Payment::toJSON()`
- `Review::toJSON()`
- `Announcement::toJSON()`
- `Coupon::toJSON()`
- `Banner::toJSON()`

These methods make it easy to send object data to the frontend.


## 15. `DatabaseManager` as an OOP component

`DatabaseManager` is a class that groups together database and file handling logic.

It uses:
- private helper methods such as `split()`, `getCurrentTimeStr()`, and `hashPassword()`.
- public methods such as `registerCustomer()`, `loginUser()`, `placeOrder()`, and `getMenu()`.

This class shows the OOP principle of separation of responsibilities: it handles only persistence, while `backend.cpp` handles request routing.


## 16. Why `User` uses a virtual destructor

The base class `User` has:
- `virtual ~User()`

This is important because when code deletes a derived object through a base pointer, the correct derived destructor runs.

Example:
```cpp
User* user = db.loginUser(username, password);
delete user;
```

If `~User()` were not virtual, `Customer` or `Admin` destructors might not run correctly.


## 17. Example of runtime behavior

### Login flow
1. Frontend sends login data to backend.
2. `DatabaseManager::loginUser()` reads the stored users.
3. If the credentials match, it creates either a `Customer` or `Admin` object.
4. The object is returned as `User*`.
5. `user->toJSON()` is called.
6. The correct overridden method produces the full JSON.

This uses inheritance + polymorphism + virtual functions.


## 18. Example of customer-wallet relationship

- `Customer` inherits from `User`.
- `Customer` contains a `Wallet`.
- The `Wallet` object is created when a `Customer` is constructed.
- `Wallet` methods like `addMoney()` and `deductMoney()` update the wallet state.

This uses both inheritance and composition.


## 19. Example of `Order` logic

`Order` stores order details such as:
- `orderId`
- `customerId`
- `items`
- `totalAmount`
- `status`
- `preparationTime`
- `paymentMethod`

It also includes behavior:
- `getProgressPercentage()` converts order status into progress.
- `toJSON()` serializes order details.

This is a classic example of an object representing both data and behavior.


## 20. Important OOP lines to remember

### In `backend/classes.h`
- `class Wallet { ... }` — defines the wallet object.
- `class User { protected: ...; virtual string getRole() const = 0; ... }` — abstract base class.
- `class Customer : public User { ... }` — inherited customer type.
- `class Admin : public User { ... }` — inherited admin type.
- `Wallet wallet;` inside `Customer` — composition.
- `virtual ~User()` — safe polymorphic cleanup.
- `string toJSON() const override` — method overriding.

### In `backend/backend.cpp`
- `DatabaseManager db;` — create database manager object.
- `User* user = db.loginUser(username, password);` — polymorphic user login.
- `response << user->toJSON();` — runtime selection of correct JSON method.


## 21. How to explain it in an answer

If your teacher asks what is used here, say:
- "We use classes like `Wallet`, `User`, `Customer`, `Admin`, `Order`, and `MenuItem`.
- "We use encapsulation because important data fields are private or protected and can only be changed by methods."
- "`Customer` and `Admin` inherit from `User`, so they reuse common code.
- "`User` is abstract and forces child classes to implement `getRole()`.
- "Polymorphism allows `User*` to refer to either a `Customer` or `Admin` and still call the right `toJSON()`.
- "Composition appears where a `Customer` has a `Wallet`.
- "Constructors set up the object state and destructors clean up when done."


## 22. Practical tips for live code changes

If you need to change a feature during live review:
1. Identify the class representing the feature.
2. Add or update fields in that class.
3. Update the constructor and getters/setters.
4. Update `toJSON()` so frontend data remains correct.
5. If persistence is affected, update `DatabaseManager` or the Python simulation.
6. If the change affects requests, update `backend.cpp` route handlers or the JS offline simulator.


## 23. Summary of OOP concepts in one sentence

This project uses classes and objects to model users, wallets, orders, and menu items; encapsulates data with private fields and public methods; reuses code with inheritance between `User`, `Customer`, and `Admin`; hides implementation details with abstraction; changes behavior dynamically with polymorphism and virtual methods; and connects objects through composition.
