-- =================================================================
-- STC Ordering Management System
-- MySQL Database Schema
-- =================================================================

CREATE DATABASE IF NOT EXISTS stc_ordering_db;
USE stc_ordering_db;

-- 1. Users Table
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    email VARCHAR(100) NOT NULL,
    role VARCHAR(20) NOT NULL DEFAULT 'customer',
    phone VARCHAR(20),
    address TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 2. Wallets Table
CREATE TABLE IF NOT EXISTS wallets (
    customer_id INT PRIMARY KEY,
    balance DECIMAL(10,2) NOT NULL DEFAULT 100.00,
    FOREIGN KEY (customer_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 3. Wallet Transactions Table
CREATE TABLE IF NOT EXISTS wallet_transactions (
    id INT AUTO_INCREMENT PRIMARY KEY,
    customer_id INT NOT NULL,
    type VARCHAR(20) NOT NULL, -- 'DEPOSIT' or 'PAYMENT'
    amount DECIMAL(10,2) NOT NULL,
    description TEXT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (customer_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 4. Menu Table
CREATE TABLE IF NOT EXISTS menu (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    price DECIMAL(10,2) NOT NULL,
    category VARCHAR(50) NOT NULL,
    is_sold_out TINYINT(1) DEFAULT 0,
    rating DECIMAL(2,1) DEFAULT 5.0,
    reviews_count INT DEFAULT 0
);

-- 5. Orders Table
CREATE TABLE IF NOT EXISTS orders (
    id INT AUTO_INCREMENT PRIMARY KEY,
    customer_id INT NOT NULL,
    total_amount DECIMAL(10,2) NOT NULL,
    status VARCHAR(50) NOT NULL DEFAULT 'Order Received', -- 'Order Received', 'Preparing', 'Ready', 'Completed'
    preparation_time INT DEFAULT 15, -- in minutes
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    payment_method VARCHAR(20) NOT NULL DEFAULT 'Wallet', -- 'Wallet' or 'COD'
    FOREIGN KEY (customer_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 6. Order Items Table
CREATE TABLE IF NOT EXISTS order_items (
    id INT AUTO_INCREMENT PRIMARY KEY,
    order_id INT NOT NULL,
    menu_item_id INT NOT NULL,
    quantity INT NOT NULL DEFAULT 1,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    FOREIGN KEY (menu_item_id) REFERENCES menu(id) ON DELETE CASCADE
);

-- 7. Notifications Table
CREATE TABLE IF NOT EXISTS notifications (
    id INT AUTO_INCREMENT PRIMARY KEY,
    customer_id INT NOT NULL,
    message TEXT NOT NULL,
    is_read TINYINT(1) DEFAULT 0,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (customer_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 8. Reviews Table
CREATE TABLE IF NOT EXISTS reviews (
    id INT AUTO_INCREMENT PRIMARY KEY,
    customer_id INT NOT NULL,
    menu_item_id INT NOT NULL,
    rating INT NOT NULL CHECK(rating >= 1 AND rating <= 5),
    comment TEXT,
    username VARCHAR(50) NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (customer_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (menu_item_id) REFERENCES menu(id) ON DELETE CASCADE
);

-- 9. Favorites Table
CREATE TABLE IF NOT EXISTS favorites (
    customer_id INT NOT NULL,
    menu_item_id INT NOT NULL,
    PRIMARY KEY (customer_id, menu_item_id),
    FOREIGN KEY (customer_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (menu_item_id) REFERENCES menu(id) ON DELETE CASCADE
);

-- 10. Announcements Table
CREATE TABLE IF NOT EXISTS announcements (
    id INT AUTO_INCREMENT PRIMARY KEY,
    message TEXT NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 11. Coupons Table
CREATE TABLE IF NOT EXISTS coupons (
    code VARCHAR(50) PRIMARY KEY,
    discount_percent DECIMAL(5,2) NOT NULL,
    is_active TINYINT(1) DEFAULT 1
);

-- 12. Banners Table
CREATE TABLE IF NOT EXISTS banners (
    id INT AUTO_INCREMENT PRIMARY KEY,
    banner_text TEXT NOT NULL,
    banner_image_url VARCHAR(255) NOT NULL,
    is_active TINYINT(1) DEFAULT 1
);

-- =================================================================
-- INSERT DEFAULT SEED DATA
-- =================================================================

-- Default Users:
-- Admin: admin / admin123 (hashed: 5381 -> hex 1ea07c164)
-- Customer: customer / customer123 (hashed: 5381 -> hex 2d7b56d3)
-- (Note: Hashing is done in C++ code using custom djb2 variant. 
-- In real SQL setups, insert values hashed by the backend register or using compatible hashes.)

INSERT INTO users (id, username, password, email, role, phone, address) VALUES
(1, 'admin', '1631de18f7027c00', 'admin@stc.com', 'admin', '+966500000001', 'STC Head Office, Riyadh'),
(2, 'customer', '933bf0b28499ba5d', 'customer@stc.com', 'customer', '+966500000002', 'Olaya District, Riyadh');

INSERT INTO wallets (customer_id, balance) VALUES
(2, 100.00);

INSERT INTO wallet_transactions (customer_id, type, amount, description) VALUES
(2, 'DEPOSIT', 100.00, 'Welcome Balance');

INSERT INTO menu (id, name, description, price, category, is_sold_out, rating, reviews_count) VALUES
(1, 'STC Signature Burger', 'Premium beef patty with cheddar cheese and STC orange sauce.', 8.99, 'Burgers', 0, 4.8, 124),
(2, 'Crispy Chicken Burger', 'Crispy fried chicken breast, lettuce, and spicy mayo in a brioche bun.', 7.49, 'Burgers', 0, 4.6, 98),
(3, 'STC Deluxe Pizza', 'Stone-baked pizza loaded with pepperoni, beef, and mozzarella.', 12.99, 'Pizza', 0, 4.9, 142),
(4, 'Margarita Flatbread', 'Simplicity at its best: tomato sauce, mozzarella, and fresh basil.', 9.50, 'Pizza', 0, 4.2, 53),
(5, 'Golden French Fries', 'Crispy golden fries seasoned with a light touch of sea salt.', 3.00, 'Sides', 0, 4.5, 210),
(6, 'STC Dynamite Wings', 'Eight pieces of crispy chicken wings tossed in signature sweet glaze.', 5.99, 'Sides', 1, 4.7, 76),
(7, 'Iced Latte', 'Chilled espresso poured over creamy whole milk and ice, sweetened.', 4.50, 'Drinks', 0, 4.4, 85),
(8, 'Fresh Orange Juice', '100% natural, freshly squeezed juice, served ice cold.', 3.50, 'Drinks', 0, 4.9, 115),
(9, 'Chocolate Lava Cake', 'Warm chocolate cake with a molten fudge center and vanilla ice cream.', 6.50, 'Desserts', 0, 4.8, 189);

INSERT INTO coupons (code, discount_percent, is_active) VALUES
('STC20', 20.00, 1),
('STC10', 10.00, 1),
('WELCOME', 15.00, 1);

INSERT INTO banners (id, banner_text, banner_image_url, is_active) VALUES
(1, 'Order STC Signature Feast - Save 20% today with Coupon: STC20!', '/images/stc_building.jpg', 1);

INSERT INTO announcements (id, message) VALUES
(1, 'Welcome to the new STC Ordering System! Use code STC20 for 20% off all orders!');

INSERT INTO reviews (id, customer_id, menu_item_id, rating, comment, username) VALUES
(1, 2, 1, 5, 'Best burger I have ever had! STC sauce is incredible.', 'customer'),
(2, 2, 3, 5, 'A bit spicy but very tasty, plenty of pepperoni.', 'customer');

INSERT INTO favorites (customer_id, menu_item_id) VALUES
(2, 1),
(2, 3);
