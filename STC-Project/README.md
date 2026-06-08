# 💜 STC Express - Full Featured Ordering & Management System

Welcome to **STC Express**, a high-fidelity, premium restaurant ordering and management web application designed with core Object-Oriented Programming (OOP) concepts in C++ CGI backend and modern vanilla HTML5/CSS3/JS frontend. 

This project incorporates the uploaded photograph of **STC Riyadh Headquarters** directly into its banners, and features deep corporate STC brand styling (deep purple `#4f008c` and coral pink `#ff007f`). It is specifically optimized to be beginner-friendly, educational, and easy to run/maintain for students and instructors alike.

---

## 🚀 Quick Start (Zero-Config Development Server)

No complex configuration is needed to run and explore the project! The system features an intelligent **Dual-Engine Gateway** which automatically starts in high-fidelity simulation mode if you don't have a C++ compiler set up, and seamlessly switches to the compiled C++ binary once compiled.

### Steps to Run:
1. **Start the Web Server**:
   Open a terminal in the project folder and run:
   ```cmd
   py server.py
   ```
   *(Alternative: `python server.py` or `npm run dev`)*
2. **Access the Website**:
   Open your browser and navigate to:
   [http://localhost:8000](http://localhost:8000)

---

## 🔑 Default Login Credentials
To help you explore the features right away, the database is pre-seeded with two accounts:

| Role | Username | Password | Purpose |
| :--- | :--- | :--- | :--- |
| **Customer** | `customer` | `customer123` | Load wallet, checkout cart, write reviews, check notifications. |
| **Admin** | `admin` | `admin123` | Add menu items, change statuses, manage coupons, view analytics. |

---

## 🎨 Premium Features Included

### 1. Customer Dashboard (`customer.html`)
- **E-Wallet Loading (STC Pay)**: Simulated Credit Card loader and QR Code scanner authorization.
- **Dynamic Menu Grid**: Search, category filters, and an interactive **dishes favoriting** toggle.
- **Premium Checkout Cart**: Adjust item quantities, apply promo codes (try `STC20` for 20% off), and choose between E-Wallet or Cash on Delivery.
- **Real-Time Progress Tracker**: Visual stepper showing preparation status ('Order Received' -> 'Preparing' -> 'Ready' -> 'Completed') with dynamic ETAs.
- **Smartphone-style Notification Inbox**: Live envelope indicators showing order stage updates and wallet transaction receipts.
- **Review & Rating System**: Double star rating select modals to submit item feedback.
- **Floating Smart Support Chat**: Contextual simulated AI agent answering FAQs.

### 2. Admin Control Panel (`admin.html`)
- **Live Sales Analytics**: Real-time sales stats, category revenue bar graphs, and checkout charts.
- **Order Pipeline Control**: Interactive status updates and estimated time adjusts.
- **Inventory Manager**: Add new dishes, edit descriptions, adjust prices, and toggle "Sold Out" switches.
- **Promo Coupon Center**: Create custom promo coupon codes and toggle coupon active states.
- **Announcement Broadcaster Center**: Post scrolling marquee announcements on the homepage.
- **Banner Customizer**: Edit marketing headline texts and backdrop image URLs.
- **Customer Directory**: Inspect user emails, phones, and live STC Pay balances.

---

## 📁 Project Architecture & OOP Structure

The backend code is written using clean, beginner-friendly standard C++11 OOP principles:
- **Encapsulation & Models** ([classes.h](file:///C:/Users/hp/Desktop/oop%20project%20web/STC-Project/backend/classes.h)): Defines `User`, `Customer` (inherits from `User`), `Admin` (inherits from `User`), `Wallet`, `MenuItem`, `Order`, `Notification`, `Review`, `Announcement`, `Coupon`, and `Banner` entities.
- **Polymorphism**: Demonstrates virtual destructors and overridden `toJSON()` and `getRole()` serializers.
- **Dual-Database Persistence** ([database.h](file:///C:/Users/hp/Desktop/oop%20project%20web/STC-Project/backend/database.h)): Reads and writes to standard, readable **flat text files** by default, but contains direct headers and switch triggers (`USE_MYSQL`) for actual MySQL database connectors.
- **CGI Controller** ([backend.cpp](file:///C:/Users/hp/Desktop/oop%20project%20web/STC-Project/backend/backend.cpp) & [cgi_helper.h](file:///C:/Users/hp/Desktop/oop%20project%20web/STC-Project/backend/cgi_helper.h)): Parses query strings and maps incoming HTTP actions to C++ model operations.

---

## 🛠️ C++ Compilation Instructions

When you are ready to compile and run the backend purely in compiled C++ binary mode:
1. Open the project folder.
2. Run the automated script:
   ```cmd
   compile.bat
   ```
   *The script will automatically detect both `g++` (MinGW) and Visual Studio C++ Compiler tools (`cl.exe`), configure the variables, compile, and place the executable inside `cgi-bin/backend.exe`.*
3. Restart `server.py`. The Python web server will instantly detect the compiled C++ executable and automatically delegate all incoming APIs to it!

---

*stc express Ltd. © 2026. Riyadh Headquarters Olaya Campus Saudi Arabia.*
