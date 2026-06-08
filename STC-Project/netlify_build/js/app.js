/* =================================================================
   STC Ordering Management System - Client Side JavaScript Logic
   Covers: Auth, Session, Wallet, Cart, Tracker, Reviews, Support Chat,
           Announcements Ticker, and Admin Controls.
   Supported: Offline Netlify Mode (seamless local storage simulation fallback)
   ================================================================= */

// Server URL configurations
const API_URL = '/cgi-bin/backend.exe';

// Detect offline simulation mode (Netlify / Static server without CGI)
const isOfflineMode = !window.location.hostname.includes('localhost') &&
    !window.location.hostname.includes('127.0.0.1');

// Global App State
let appState = {
    user: null,
    menu: [],
    favorites: [],
    cart: [],
    selectedCategory: 'All',
    searchQuery: '',
    currentOrder: null, // For tracking active order
    walletMethod: 'CreditCard',
    activeTab: 'menu',
    activeAdminTab: 'analytics',
    couponDiscount: 0.0,
    appliedCoupon: '',
    selectedPaymentMethod: 'Wallet',
    chatOpen: false,
    starRating: 5
};

// Appetite-inducing Stock Photos from Unsplash
const ITEM_IMAGES = {
    1: 'https://images.unsplash.com/photo-1568901346375-23c9450c58cd?w=500&auto=format&fit=crop', // Burger
    2: 'https://images.unsplash.com/photo-1625813506062-0aeb1d7a094b?w=500&auto=format&fit=crop', // Crispy Burger
    3: 'https://images.unsplash.com/photo-1513104890138-7c749659a591?w=500&auto=format&fit=crop', // Pizza
    4: 'https://images.unsplash.com/photo-1574071318508-1cdbab80d002?w=500&auto=format&fit=crop', // Flatbread
    5: 'https://images.unsplash.com/photo-1573080496219-bb080dd4f877?w=500&auto=format&fit=crop', // Fries
    6: 'https://images.unsplash.com/photo-1567620832903-9fc6debc209f?w=500&auto=format&fit=crop', // Wings
    7: 'https://images.unsplash.com/photo-1517701604599-bb29b565090c?w=500&auto=format&fit=crop', // Latte
    8: 'https://images.unsplash.com/photo-1621506289937-a8e4df240d0b?w=500&auto=format&fit=crop', // Orange Juice
    9: 'https://images.unsplash.com/photo-1606313564200-e75d5e30476c?w=500&auto=format&fit=crop'  // Lava Cake
};

function getMenuItemImage(item) {
    if (ITEM_IMAGES[item.id]) return ITEM_IMAGES[item.id];
    // Fallback by Category
    switch (item.category) {
        case 'Burgers': return 'https://images.unsplash.com/photo-1568901346375-23c9450c58cd?w=500&auto=format&fit=crop';
        case 'Pizza': return 'https://images.unsplash.com/photo-1513104890138-7c749659a591?w=500&auto=format&fit=crop';
        case 'Sides': return 'https://images.unsplash.com/photo-1573080496219-bb080dd4f877?w=500&auto=format&fit=crop';
        case 'Drinks': return 'https://images.unsplash.com/photo-1517701604599-bb29b565090c?w=500&auto=format&fit=crop';
        case 'Desserts': return 'https://images.unsplash.com/photo-1606313564200-e75d5e30476c?w=500&auto=format&fit=crop';
        default: return 'https://images.unsplash.com/photo-1546069901-ba9599a7e63c?w=500&auto=format&fit=crop';
    }
}

// Format prices with PKR Rs. Symbol
function formatCurrency(amount) {
    return `Rs. ${parseFloat(amount).toLocaleString(undefined, { minimumFractionDigits: 2, maximumFractionDigits: 2 })}`;
}

// =================================================================
// 1. APP INITIALIZATION & STARTUP
// =================================================================
document.addEventListener('DOMContentLoaded', () => {
    initTheme();
    loadSession();
    fetchAnnouncements();
    detectPageContext();

    if (isOfflineMode) {
        console.log('[INFO] STC Express is running in Netlify Offline Mock Mode. Persistence is simulated via LocalStorage.');
    }
});

// Detect which page we are on and boot corresponding views
function detectPageContext() {
    const path = window.location.pathname;

    if (path.includes('customer.html')) {
        // Customer page logic
        if (!appState.user || appState.user.role !== 'customer') {
            alert('Unauthorized access. Please login first.');
            window.location.href = '/';
            return;
        }
        setupCustomerDashboard();
    } else if (path.includes('admin.html')) {
        // Admin page logic
        if (!appState.user || appState.user.role !== 'admin') {
            alert('Unauthorized access. Please login first.');
            window.location.href = '/';
            return;
        }
        setupAdminDashboard();
    } else {
        // Homepage logic
        loadPublicMenu();
        updatePublicNav();
        fetchBanners();
    }
}

// Initialize Theme (Dark/Light Mode)
function initTheme() {
    const savedTheme = localStorage.getItem('stc_theme') || 'light';
    document.documentElement.setAttribute('data-theme', savedTheme);

    const themeBtn = document.getElementById('theme-toggle-btn');
    if (themeBtn) {
        updateThemeIcon(savedTheme);
        themeBtn.addEventListener('click', toggleTheme);
    }
}

function toggleTheme() {
    const currentTheme = document.documentElement.getAttribute('data-theme');
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark';

    document.documentElement.setAttribute('data-theme', newTheme);
    localStorage.setItem('stc_theme', newTheme);
    updateThemeIcon(newTheme);
}

function updateThemeIcon(theme) {
    const themeBtn = document.getElementById('theme-toggle-btn');
    if (!themeBtn) return;
    if (theme === 'dark') {
        themeBtn.innerHTML = '<i class="fa-solid fa-sun" style="color: var(--stc-yellow);"></i>';
    } else {
        themeBtn.innerHTML = '<i class="fa-solid fa-moon"></i>';
    }
}

// Load session from localStorage
function loadSession() {
    const savedUser = localStorage.getItem('stc_user');
    if (savedUser) {
        appState.user = JSON.parse(savedUser);
    }
}

// Save session
function saveSession(userData) {
    appState.user = userData;
    localStorage.setItem('stc_user', JSON.stringify(userData));
}

// Logout helper
function handleLogout() {
    localStorage.removeItem('stc_user');
    appState.user = null;
    window.location.href = '/';
}

// =================================================================
// 2. CORE REST CLIENT (CGI & OFFLINE LOCALSTORAGE SIMULATOR)
// =================================================================
async function makeRequest(action, method = 'GET', body = null) {
    // If not running locally, execute Netlify Client-Side Simulatation
    if (isOfflineMode) {
        return new Promise((resolve, reject) => {
            setTimeout(() => {
                try {
                    const res = simulateOfflineCgi(action, method, body);
                    resolve(res);
                } catch (e) {
                    reject(e);
                }
            }, 300); // 300ms network delay simulation
        });
    }

    // Normal CGI Execution on localhost
    try {
        const url = `${API_URL}?action=${action}`;
        const options = {
            method: method,
            headers: {}
        };

        if (body) {
            options.headers['Content-Type'] = 'application/json';
            options.body = JSON.stringify(body);
        }

        const response = await fetch(url, options);

        if (!response.ok) {
            const errData = await response.json().catch(() => ({}));
            throw new Error(errData.message || `CGI Request Failed: ${response.status}`);
        }

        return await response.json();
    } catch (error) {
        console.error('Request Error:', error);
        throw error;
    }
}

// =================================================================
// 3. OFFLINE LOCALSTORAGE DATABASE SIMULATION ENGINE (Netlify Only)
// =================================================================
function simulateOfflineCgi(action, method, body) {
    const defaultMenu = [
        { id: 1, name: "STC Signature Burger", description: "Premium beef patty with cheddar cheese and STC orange sauce.", price: 1200.00, category: "Burgers", is_sold_out: false, rating: 4.8, reviews_count: 124 },
        { id: 2, name: "Crispy Chicken Burger", description: "Crispy fried chicken breast, lettuce, and spicy mayo in a brioche bun.", price: 950.00, category: "Burgers", is_sold_out: false, rating: 4.6, reviews_count: 98 },
        { id: 3, name: "STC Deluxe Pizza", description: "Stone-baked pizza loaded with pepperoni, beef, and mozzarella.", price: 2200.00, category: "Pizza", is_sold_out: false, rating: 4.9, reviews_count: 142 },
        { id: 4, name: "Margarita Flatbread", description: "Simplicity at its best: tomato sauce, mozzarella, and fresh basil.", price: 1500.00, category: "Pizza", is_sold_out: false, rating: 4.2, reviews_count: 53 },
        { id: 5, name: "Golden French Fries", description: "Crispy golden fries seasoned with a light touch of sea salt.", price: 450.00, category: "Sides", is_sold_out: false, rating: 4.5, reviews_count: 210 },
        { id: 6, name: "STC Dynamite Wings", description: "Eight pieces of crispy chicken wings tossed in signature sweet glaze.", price: 750.00, category: "Sides", is_sold_out: true, rating: 4.7, reviews_count: 76 },
        { id: 7, name: "Iced Latte", description: "Chilled espresso poured over creamy whole milk and ice, sweetened.", price: 650.00, category: "Drinks", is_sold_out: false, rating: 4.4, reviews_count: 85 },
        { id: 8, name: "Fresh Orange Juice", description: "100% natural, freshly squeezed juice, served ice cold.", price: 500.00, category: "Drinks", is_sold_out: false, rating: 4.9, reviews_count: 115 },
        { id: 9, name: "Chocolate Lava Cake", description: "Warm chocolate cake with a molten fudge center and vanilla ice cream.", price: 850.00, category: "Desserts", is_sold_out: false, rating: 4.8, reviews_count: 189 }
    ];

    // Seed mock local storage databases
    if (!localStorage.getItem('stc_sim_users')) {
        localStorage.setItem('stc_sim_users', JSON.stringify([
            { id: 1, username: "admin", password: "admin123", email: "admin@stc.com", role: "admin" },
            { id: 2, username: "customer", password: "customer123", email: "customer@stc.com", role: "customer", phone: "+966500000002", address: "Olaya District, Riyadh", wallet: { balance: 0.00, transactions: ["Account Created (DEPOSIT): +PKR 0.00 on 2026-05-29 21:00:00"] } }
        ]));
    }
    if (!localStorage.getItem('stc_sim_menu')) {
        localStorage.setItem('stc_sim_menu', JSON.stringify(defaultMenu));
    }
    if (!localStorage.getItem('stc_sim_orders')) {
        localStorage.setItem('stc_sim_orders', JSON.stringify([]));
    }
    if (!localStorage.getItem('stc_sim_notifications')) {
        localStorage.setItem('stc_sim_notifications', JSON.stringify([]));
    }
    if (!localStorage.getItem('stc_sim_favorites')) {
        localStorage.setItem('stc_sim_favorites', JSON.stringify([2, 1]));
    }
    if (!localStorage.getItem('stc_sim_coupons')) {
        localStorage.setItem('stc_sim_coupons', JSON.stringify([
            { code: "STC20", discount_percent: 20.00, is_active: 1 },
            { code: "STC10", discount_percent: 10.00, is_active: 1 },
            { code: "WELCOME", discount_percent: 15.00, is_active: 1 }
        ]));
    }
    if (!localStorage.getItem('stc_sim_banners')) {
        localStorage.setItem('stc_sim_banners', JSON.stringify([
            { id: 1, banner_text: "Order STC Signature Feast - Save 20% today with Coupon: STC20!", banner_image_url: "/images/stc_building.jpg", is_active: 1 }
        ]));
    }
    if (!localStorage.getItem('stc_sim_announcements')) {
        localStorage.setItem('stc_sim_announcements', JSON.stringify([
            { id: 1, message: "Welcome to the new STC Ordering System! Use code STC20 for 20% off all orders!", timestamp: "2026-05-29 21:00:00" }
        ]));
    }
    if (!localStorage.getItem('stc_sim_reviews')) {
        localStorage.setItem('stc_sim_reviews', JSON.stringify([
            { id: 1, customerId: 2, menuItemId: 1, rating: 5, comment: "Best burger I have ever had! STC sauce is incredible.", username: "customer", timestamp: "2026-05-29 21:00:00" },
            { id: 2, customerId: 2, menuItemId: 3, rating: 5, comment: "A bit spicy but very tasty, plenty of pepperoni.", username: "customer", timestamp: "2026-05-29 21:01:00" }
        ]));
    }

    const users = JSON.parse(localStorage.getItem('stc_sim_users'));
    const menu = JSON.parse(localStorage.getItem('stc_sim_menu'));
    const orders = JSON.parse(localStorage.getItem('stc_sim_orders'));
    const notifs = JSON.parse(localStorage.getItem('stc_sim_notifications'));
    const favs = JSON.parse(localStorage.getItem('stc_sim_favorites'));
    const coupons = JSON.parse(localStorage.getItem('stc_sim_coupons'));
    const banners = JSON.parse(localStorage.getItem('stc_sim_banners'));
    const announces = JSON.parse(localStorage.getItem('stc_sim_announcements'));
    const reviews = JSON.parse(localStorage.getItem('stc_sim_reviews'));

    const now_str = new Date().toISOString().replace('T', ' ').substring(0, 19);

    // --- Action Routing ---
    if (action === "get_menu") {
        return menu;
    }

    else if (action === "login" && method === "POST") {
        const u = users.find(user => user.username === body.username && user.password === body.password);
        if (u) return { status: "success", user: u };
        return { status: "error", message: "Invalid username or password." };
    }

    else if (action === "register" && method === "POST") {
        const exists = users.find(user => user.username === body.username);
        if (exists) return { status: "error", message: "Username already exists." };
        const new_id = users.length + 1;
        const new_user = {
            id: new_id,
            username: body.username,
            password: body.password,
            role: "customer",
            phone: body.phone || "",
            address: body.address || "",
            wallet: { balance: 0.00, transactions: [`Account Created (DEPOSIT): +PKR 0.00 on ${now_str}`] }
        };
        users.push(new_user);
        localStorage.setItem('stc_sim_users', JSON.stringify(users));
        return { status: "success", message: "Registration successful! Your wallet balance is PKR 0.00." };
    }

    else if (action === "place_order" && method === "POST") {
        const cust = users.find(user => user.id === parseInt(body.customerId));
        if (!cust) return { status: "error", message: "Customer not found." };
        const total = parseFloat(body.totalAmount);

        if (body.paymentMethod === "Wallet") {
            if (cust.wallet.balance < total) {
                return { status: "error", message: "Transaction failed. Check wallet balance." };
            }
            cust.wallet.balance -= total;
            cust.wallet.transactions.unshift(`Order checkout payment (PAYMENT): -PKR ${total.toFixed(2)} on ${now_str}`);
            localStorage.setItem('stc_sim_users', JSON.stringify(users));
        }

        const new_order_id = orders.length + 1;
        const new_order = {
            orderId: new_order_id,
            customerId: cust.id,
            total_amount: total,
            status: "Order Received",
            preparation_time: 15,
            timestamp: now_str,
            items: body.items,
            payment_method: body.paymentMethod
        };
        orders.push(new_order);
        localStorage.setItem('stc_sim_orders', JSON.stringify(orders));

        // Create notification for customer
        const notif_id = notifs.length + 1;
        notifs.unshift({
            id: notif_id,
            customerId: cust.id,
            message: `Your order #${new_order_id} has been received! Payment Mode: ${body.paymentMethod} - Total: PKR ${total.toFixed(2)}`,
            is_read: 0,
            timestamp: now_str
        });

        // Create notification for admin (id=1)
        notifs.unshift({
            id: notif_id + 1,
            customerId: 1,
            message: `New Order #${new_order_id} from ${cust.username}! Items: ${body.items} - Total: PKR ${total.toFixed(2)} - Payment: ${body.paymentMethod}`,
            is_read: 0,
            timestamp: now_str
        });
        localStorage.setItem('stc_sim_notifications', JSON.stringify(notifs));

        return { status: "success", orderId: new_order_id };
    }

    else if (action.startsWith("get_order")) {
        const urlParams = new URLSearchParams(action);
        const oid = parseInt(urlParams.get('orderId'));
        const order = orders.find(o => o.orderId === oid);
        if (order) return order;
        return { status: "error", message: "Order not found." };
    }

    else if (action.startsWith("get_customer_orders")) {
        const urlParams = new URLSearchParams(action);
        const cid = parseInt(urlParams.get('customerId'));
        return orders.filter(o => o.customerId === cid).reverse();
    }

    else if (action === "add_wallet_balance" && method === "POST") {
        const cust = users.find(user => user.id === parseInt(body.customerId));
        if (cust) {
            const amt = parseFloat(body.amount);
            cust.wallet.balance += amt;
            cust.wallet.transactions.unshift(`Loaded money via ${body.method} (DEPOSIT): +PKR ${amt.toFixed(2)} on ${now_str}`);
            localStorage.setItem('stc_sim_users', JSON.stringify(users));

            // Create notification
            notifs.unshift({
                id: notifs.length + 1,
                customerId: cust.id,
                message: `Your E-Wallet has been loaded with PKR ${amt.toFixed(2)} via ${body.method}.`,
                is_read: 0,
                timestamp: now_str
            });
            localStorage.setItem('stc_sim_notifications', JSON.stringify(notifs));
            return { status: "success", message: "Money successfully added." };
        }
        return { status: "error", message: "Customer not found." };
    }

    else if (action.startsWith("get_notifications")) {
        const urlParams = new URLSearchParams(action);
        const cid = parseInt(urlParams.get('customerId'));
        return notifs.filter(n => n.customerId === cid);
    }

    else if (action === "mark_notifications_read" && method === "POST") {
        const cid = parseInt(body.customerId);
        notifs.forEach(n => {
            if (n.customerId === cid) n.is_read = 1;
        });
        localStorage.setItem('stc_sim_notifications', JSON.stringify(notifs));
        return { status: "success" };
    }

    else if (action === "add_menu_item" && method === "POST") {
        const new_id = menu.length + 1;
        menu.push({
            id: new_id,
            name: body.name,
            description: body.description,
            price: parseFloat(body.price),
            category: body.category,
            is_sold_out: false,
            rating: 5.0,
            reviews_count: 0
        });
        localStorage.setItem('stc_sim_menu', JSON.stringify(menu));
        return { status: "success", message: "Menu item added!" };
    }

    else if (action === "update_menu_item" && method === "POST") {
        const item = menu.find(m => m.id === parseInt(body.id));
        if (item) {
            item.name = body.name;
            item.description = body.description;
            item.price = parseFloat(body.price);
            item.category = body.category;
            item.is_sold_out = body.is_sold_out === "1" || body.is_sold_out === true;
            localStorage.setItem('stc_sim_menu', JSON.stringify(menu));
            return { status: "success", message: "Item updated successfully." };
        }
        return { status: "error", message: "Item not found." };
    }

    else if (action === "delete_menu_item" && method === "POST") {
        const index = menu.findIndex(m => m.id === parseInt(body.id));
        if (index !== -1) {
            menu.splice(index, 1);
            localStorage.setItem('stc_sim_menu', JSON.stringify(menu));
            return { status: "success", message: "Item deleted successfully." };
        }
        return { status: "error", message: "Item not found." };
    }

    else if (action === "set_sold_out" && method === "POST") {
        const item = menu.find(m => m.id === parseInt(body.id));
        if (item) {
            item.is_sold_out = body.is_sold_out === "1" || body.is_sold_out === true;
            localStorage.setItem('stc_sim_menu', JSON.stringify(menu));
            return { status: "success", message: "Sold out status updated." };
        }
        return { status: "error", message: "Failed to update." };
    }

    else if (action === "get_all_orders") {
        return orders.reverse();
    }

    else if (action === "update_order_status" && method === "POST") {
        const o = orders.find(order => order.orderId === parseInt(body.orderId));
        if (o) {
            o.status = body.status;
            localStorage.setItem('stc_sim_orders', JSON.stringify(orders));

            // Create notification
            notifs.unshift({
                id: notifs.length + 1,
                customerId: o.customerId,
                message: `Your order #${o.orderId} status has been updated to: ${body.status}`,
                is_read: 0,
                timestamp: now_str
            });
            localStorage.setItem('stc_sim_notifications', JSON.stringify(notifs));
            return { status: "success", message: "Order status updated!" };
        }
        return { status: "error", message: "Order not found." };
    }

    else if (action === "update_prep_time" && method === "POST") {
        const o = orders.find(order => order.orderId === parseInt(body.orderId));
        if (o) {
            o.preparation_time = parseInt(body.minutes);
            localStorage.setItem('stc_sim_orders', JSON.stringify(orders));
            return { status: "success", message: "Estimated time updated." };
        }
        return { status: "error", message: "Order not found." };
    }

    else if (action === "get_customers") {
        return users.filter(u => u.role === "customer");
    }

    else if (action.startsWith("get_reviews")) {
        const urlParams = new URLSearchParams(action);
        const item_id = parseInt(urlParams.get('menuItemId'));
        if (!isNaN(item_id)) {
            return reviews.filter(r => r.menuItemId === item_id).reverse();
        }
        return reviews.reverse();
    }

    else if (action === "add_review" && method === "POST") {
        const cust = users.find(u => u.id === parseInt(body.customerId));
        const item = menu.find(m => m.id === parseInt(body.menuItemId));
        if (cust && item) {
            const rev_id = reviews.length + 1;
            const new_rev = {
                id: rev_id,
                customerId: cust.id,
                menuItemId: item.id,
                rating: parseInt(body.rating),
                comment: body.comment,
                username: cust.username,
                timestamp: now_str
            };
            reviews.push(new_rev);
            localStorage.setItem('stc_sim_reviews', JSON.stringify(reviews));

            // Update item rating counts
            const all_revs = reviews.filter(r => r.menuItemId === item.id);
            item.rating = all_revs.reduce((sum, r) => sum + r.rating, 0) / all_revs.length;
            item.reviews_count = all_revs.length;
            localStorage.setItem('stc_sim_menu', JSON.stringify(menu));

            return { status: "success", message: "Your feedback has been recorded!" };
        }
        return { status: "error", message: "Failed to submit review." };
    }

    else if (action === "toggle_favorite" && method === "POST") {
        const cid = parseInt(body.customerId);
        const iid = parseInt(body.menuItemId);
        const idx = favs.indexOf(iid);
        if (idx !== -1) {
            favs.splice(idx, 1);
            localStorage.setItem('stc_sim_favorites', JSON.stringify(favs));
            return { status: "success", state: "removed", message: "Removed from favorites." };
        } else {
            favs.push(iid);
            localStorage.setItem('stc_sim_favorites', JSON.stringify(favs));
            return { status: "success", state: "added", message: "Added to favorites." };
        }
    }

    else if (action.startsWith("get_favorites")) {
        return favs;
    }

    else if (action === "get_announcements") {
        return announces;
    }

    else if (action === "add_announcement" && method === "POST") {
        announces.unshift({
            id: announces.length + 1,
            message: body.message,
            timestamp: now_str
        });
        localStorage.setItem('stc_sim_announcements', JSON.stringify(announces));
        return { status: "success", message: "Announcement broadcasted successfully!" };
    }

    else if (action === "get_coupons") {
        return coupons;
    }

    else if (action === "add_coupon" && method === "POST") {
        coupons.push({
            code: body.code.toUpperCase(),
            discount_percent: parseFloat(body.discount_percent),
            is_active: 1
        });
        localStorage.setItem('stc_sim_coupons', JSON.stringify(coupons));
        return { status: "success", message: "Coupon created successfully!" };
    }

    else if (action === "toggle_coupon" && method === "POST") {
        const c = coupons.find(coupon => coupon.code === body.code);
        if (c) {
            c.is_active = body.is_active === "1" || body.is_active === true ? 1 : 0;
            localStorage.setItem('stc_sim_coupons', JSON.stringify(coupons));
            return { status: "success", message: "Coupon state updated successfully." };
        }
        return { status: "error", message: "Coupon not found." };
    }

    else if (action === "get_banners") {
        return banners;
    }

    else if (action === "update_banner" && method === "POST") {
        banners[0].banner_text = body.banner_text;
        banners[0].banner_image_url = body.banner_image_url;
        localStorage.setItem('stc_sim_banners', JSON.stringify(banners));
        return { status: "success", message: "Website banner updated!" };
    }

    else if (action === "get_sales_analytics") {
        const rev = orders.reduce((sum, o) => sum + o.total_amount, 0);
        const cat_rev = {};
        orders.forEach(o => {
            o.items.split(',').forEach(pair => {
                if (pair.includes(':')) {
                    const [iid, qty] = pair.split(':').map(Number);
                    const item = menu.find(m => m.id === iid);
                    if (item) {
                        cat_rev[item.category] = (cat_rev[item.category] || 0) + (item.price * qty);
                    }
                }
            });
        });
        const category_revenue = Object.keys(cat_rev).map(cat => ({ category: cat, revenue: cat_rev[cat] }));

        const daily = {};
        orders.forEach(o => {
            const dt = o.timestamp.split(' ')[0];
            daily[dt] = (daily[dt] || 0) + 1;
        });
        const daily_orders = Object.keys(daily).map(dt => ({ date: dt, orders_count: daily[dt] })).reverse();

        return {
            total_revenue: rev,
            total_orders: orders.length,
            total_customers: users.filter(u => u.role === "customer").length,
            category_revenue: category_revenue,
            daily_orders: daily_orders.slice(0, 5)
        };
    }
}

// =================================================================
// 4. AUTHENTICATION CONTROLS (Login/Register Modals)
// =================================================================
function openLoginModal(role = 'customer') {
    document.getElementById('login-role').value = role;

    const title = document.getElementById('login-title');
    const subtitle = document.getElementById('login-subtitle');
    const footer = document.getElementById('login-footer');

    if (role === 'admin') {
        title.innerText = 'Admin Portal Access';
        subtitle.innerText = 'STC Restaurant System Management';
        footer.style.display = 'none';
    } else {
        title.innerText = 'Sign In';
        subtitle.innerText = 'Access your STC Wallet & Orders';
        footer.style.display = 'block';
    }

    document.getElementById('login-modal').classList.add('active');
}

function toggleAuthModal(target) {
    if (target === 'register') {
        closeModal('login-modal');
        document.getElementById('register-modal').classList.add('active');
    } else {
        closeModal('register-modal');
        openLoginModal('customer');
    }
}

function closeModal(modalId) {
    document.getElementById(modalId).classList.remove('active');
}

async function handleLogin(event) {
    event.preventDefault();
    const role = document.getElementById('login-role').value;
    const username = document.getElementById('login-username').value.trim();
    const password = document.getElementById('login-password').value;

    const submitBtn = document.getElementById('btn-login-submit');
    submitBtn.innerHTML = '<i class="fa-solid fa-spinner fa-spin"></i> Authenticating...';
    submitBtn.disabled = true;

    try {
        const response = await makeRequest('login', 'POST', { username, password });
        if (response.status === 'success') {
            saveSession(response.user);
            closeModal('login-modal');

            // Redirect based on role
            if (response.user.role === 'admin') {
                window.location.href = '/admin.html';
            } else {
                window.location.href = '/customer.html';
            }
        } else {
            alert(response.message || 'Login failed.');
        }
    } catch (error) {
        alert(error.message || 'Authentication error. Double-check your server is running.');
    } finally {
        submitBtn.innerText = 'Login';
        submitBtn.disabled = false;
    }
}

async function handleRegister(event) {
    event.preventDefault();
    const username = document.getElementById('reg-username').value.trim();
    const password = document.getElementById('reg-password').value;
    const email = document.getElementById('reg-email').value.trim();
    const phone = document.getElementById('reg-phone').value.trim();
    const address = document.getElementById('reg-address').value.trim();

    try {
        const response = await makeRequest('register', 'POST', { username, password, email, phone, address });
        if (response.status === 'success') {
            alert(response.message || 'Registration successful! Log in now.');
            closeModal('register-modal');
            openLoginModal('customer');
        } else {
            alert(response.message || 'Registration failed.');
        }
    } catch (error) {
        alert(error.message || 'Error occurred during registration.');
    }
}

// Update public homepage top navigation bar based on auth session
function updatePublicNav() {
    const authContainer = document.getElementById('auth-nav-container');
    if (!authContainer) return;

    if (appState.user) {
        const panelLink = appState.user.role === 'admin' ? '/admin.html' : '/customer.html';

        authContainer.innerHTML = `
            <div class="user-profile-menu" onclick="window.location.href='${panelLink}'">
                <div class="user-avatar">${appState.user.username[0].toUpperCase()}</div>
                <span style="font-weight:600; font-size:14px;">${appState.user.username}</span>
                <i class="fa-solid fa-arrow-right-to-bracket" style="font-size:14px; opacity:0.8; margin-left: 8px;"></i>
            </div>
        `;
    }
}

// =================================================================
// 4. HOMEPAGE PUBLIC MENU CONTROLS
// =================================================================
async function loadPublicMenu() {
    const menuGrid = document.getElementById('menu-items-grid');
    if (!menuGrid) return;

    try {
        const items = await makeRequest('get_menu');
        appState.menu = items;
        renderPublicMenu();
    } catch (err) {
        menuGrid.innerHTML = `
            <div style="grid-column: 1/-1; text-align: center; color: var(--text-secondary); padding: 50px 0;">
                <i class="fa-solid fa-triangle-exclamation" style="font-size: 40px; color: var(--stc-coral);"></i>
                <p style="margin-top: 16px;">Failed to load food menu. Ensure database flat files are seeded.</p>
            </div>
        `;
    }
}

function renderPublicMenu() {
    const menuGrid = document.getElementById('menu-items-grid');
    if (!menuGrid) return;

    // Filter
    let filtered = appState.menu;
    if (appState.selectedCategory !== 'All') {
        filtered = filtered.filter(item => item.category === appState.selectedCategory);
    }
    if (appState.searchQuery) {
        const query = appState.searchQuery.toLowerCase();
        filtered = filtered.filter(item =>
            item.name.toLowerCase().includes(query) ||
            item.description.toLowerCase().includes(query)
        );
    }

    if (filtered.length === 0) {
        menuGrid.innerHTML = `
            <div style="grid-column: 1/-1; text-align: center; color: var(--text-secondary); padding: 40px 0;">
                <p>No dishes found matching your selection.</p>
            </div>
        `;
        return;
    }

    menuGrid.innerHTML = filtered.map(item => {
        const soldOutClass = item.is_sold_out ? 'sold-out' : '';
        const starRatingStr = generateStarsHTML(item.rating);
        const itemImage = getMenuItemImage(item);

        return `
            <div class="menu-card glass-card ${soldOutClass}">
                ${item.is_sold_out ? '<div class="sold-out-overlay">Sold Out</div>' : ''}
                <div class="menu-card-img-wrap">
                    <span class="menu-card-category">${item.category}</span>
                    <img src="${itemImage}" class="menu-card-img" alt="${item.name}">
                </div>
                <div class="menu-card-body">
                    <h3 class="menu-card-title">${item.name}</h3>
                    <p class="menu-card-desc">${item.description}</p>
                    <div class="menu-card-meta">
                        <div class="menu-card-rating">
                            ${starRatingStr}
                            <span class="menu-card-reviews">(${item.reviews_count} reviews)</span>
                        </div>
                    </div>
                    <div class="menu-card-meta" style="margin-bottom: 0;">
                        <span class="menu-card-price">${formatCurrency(item.price)}</span>
                        <div class="menu-card-actions">
                            <button class="btn-add-cart" onclick="triggerCartAction(${item.id})">Add to Order</button>
                        </div>
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

function triggerCartAction(itemId) {
    if (!appState.user) {
        alert('Please login or register to add items to your shopping cart.');
        openLoginModal('customer');
    } else {
        // Already logged in - redirect to customer panel, adding item in cart
        let cartItems = JSON.parse(localStorage.getItem(`stc_cart_${appState.user.id}`) || '[]');
        const existing = cartItems.find(i => i.id === itemId);
        if (existing) {
            existing.qty += 1;
        } else {
            cartItems.push({ id: itemId, qty: 1 });
        }
        localStorage.setItem(`stc_cart_${appState.user.id}`, JSON.stringify(cartItems));
        window.location.href = '/customer.html?action=cart';
    }
}

function filterCategory(cat) {
    appState.selectedCategory = cat;

    // UI active tab toggles
    const tabs = document.querySelectorAll('#menu-categories .category-tab');
    tabs.forEach(tab => {
        if (tab.innerText.trim().includes(cat) || (cat === 'All' && tab.innerText.trim().includes('All Items'))) {
            tab.classList.add('active');
        } else {
            tab.classList.remove('active');
        }
    });

    renderPublicMenu();
}

function searchMenu() {
    appState.searchQuery = document.getElementById('menu-search-input').value.trim();
    renderPublicMenu();
}

// Helpers
function getCategoryIconClass(cat) {
    switch (cat) {
        case 'Burgers': return 'fa-solid fa-hamburger';
        case 'Pizza': return 'fa-solid fa-pizza-slice';
        case 'Sides': return 'fa-solid fa-cookie-bite';
        case 'Drinks': return 'fa-solid fa-mug-hot';
        case 'Desserts': return 'fa-solid fa-ice-cream';
        default: return 'fa-solid fa-utensils';
    }
}

function generateStarsHTML(rating) {
    let stars = '';
    const rounded = Math.round(rating);
    for (let i = 1; i <= 5; ++i) {
        if (i <= rounded) {
            stars += '<i class="fa-solid fa-star"></i>';
        } else {
            stars += '<i class="fa-regular fa-star" style="opacity:0.3;"></i>';
        }
    }
    return stars;
}

// =================================================================
// 5. ANNOUNCEMENTS & TICKERS
// =================================================================
async function fetchAnnouncements() {
    const ticker = document.getElementById('announcements-ticker');
    if (!ticker) return;

    try {
        const announcements = await makeRequest('get_announcements');
        if (announcements.length > 0) {
            ticker.innerHTML = announcements.map(a => `
                <div class="ticker-item">${a.message} <span style="font-size:11px; opacity:0.6;">(${a.timestamp})</span></div>
            `).join('');
        }
    } catch (e) {
        console.warn('Failed to load scrolling announcements ticker');
    }
}

async function fetchBanners() {
    const bannerSection = document.getElementById('hero-banner');
    const bannerHeadline = document.getElementById('banner-text');
    if (!bannerSection) return;

    try {
        const banners = await makeRequest('get_banners');
        if (banners.length > 0) {
            const active = banners.find(b => b.is_active === 1 || b.is_active === true);
            if (active) {
                bannerHeadline.innerText = active.banner_text;
                bannerSection.style.backgroundImage = `url('${active.banner_image_url}')`;
            }
        }
    } catch (e) {
        console.warn('Failed to load banner settings');
    }
}

// =================================================================
// 6. CUSTOMER DASHBOARD OPERATIONS
// =================================================================
function setupCustomerDashboard() {
    // Fill Profile Info
    document.getElementById('sidebar-username').innerText = appState.user.username;
    document.getElementById('sidebar-email').innerText = appState.user.email;
    document.getElementById('sidebar-user-avatar').innerText = appState.user.username[0].toUpperCase();
    document.getElementById('nav-user-avatar').innerText = appState.user.username[0].toUpperCase();
    document.getElementById('nav-username').innerText = appState.user.username;

    // Load wallet balance from backend representation
    syncWalletBalance();

    // Check if query action directs to Cart tab
    const urlParams = new URLSearchParams(window.location.search);
    const tabAction = urlParams.get('action');
    if (tabAction === 'cart') {
        switchDashboardTab('cart');
    } else {
        switchDashboardTab('menu');
    }

    // Refresh loops
    loadCustomerMenu();
    loadFavoritesList();
    fetchNotificationsList();
    loadCustomerOrderHistory();

    // Poll active orders every 6 seconds to update Tracker
    setInterval(pollActiveOrderTracker, 6000);
}

function switchDashboardTab(tabId) {
    appState.activeTab = tabId;

    // Update Menu button state
    const tabBtns = document.querySelectorAll('.sidebar-menu-item');
    tabBtns.forEach(btn => {
        if (btn.id === `tab-btn-${tabId}`) {
            btn.classList.add('active');
        } else {
            btn.classList.remove('active');
        }
    });

    // Toggle Content Views
    const views = document.querySelectorAll('.tab-view');
    views.forEach(view => {
        if (view.id === `view-${tabId}`) {
            view.classList.add('active');
        } else {
            view.classList.remove('active');
        }
    });

    // Specific Tab Loaders
    if (tabId === 'cart') {
        renderCartView();
    } else if (tabId === 'tracker') {
        renderLiveTracker();
    } else if (tabId === 'favorites') {
        renderFavoritesList();
    } else if (tabId === 'inbox') {
        fetchNotificationsList();
    } else if (tabId === 'history') {
        loadCustomerOrderHistory();
    }
}

async function syncWalletBalance() {
    try {
        if (isOfflineMode) {
            const users = JSON.parse(localStorage.getItem('stc_sim_users') || '[]');
            const me = users.find(c => c.id === appState.user.id);
            if (me) {
                appState.user.wallet = me.wallet;
                saveSession(appState.user);
                document.getElementById('sidebar-wallet-balance').innerText = formatCurrency(me.wallet.balance);
            }
            return;
        }

        const response = await makeRequest('get_customers');
        const me = response.find(c => c.id === appState.user.id);
        if (me) {
            appState.user.wallet = me.wallet;
            saveSession(appState.user);
            document.getElementById('sidebar-wallet-balance').innerText = formatCurrency(me.wallet.balance);
        }
    } catch (e) {
        console.warn('Could not sync wallet balance from server.');
    }
}

async function loadCustomerMenu() {
    const menuGrid = document.getElementById('customer-menu-grid');
    if (!menuGrid) return;

    try {
        const items = await makeRequest('get_menu');
        appState.menu = items;
        renderCustomerMenu();
    } catch (err) {
        menuGrid.innerHTML = `<p>Failed to load kitchen menu. Ensure C++ CGI binary is compiled.</p>`;
    }
}

function renderCustomerMenu() {
    const menuGrid = document.getElementById('customer-menu-grid');
    if (!menuGrid) return;

    let filtered = appState.menu;
    const catSelect = document.getElementById('menu-category-select-cust').value;
    const query = document.getElementById('menu-search-input-cust').value.trim().toLowerCase();

    if (catSelect !== 'All') {
        filtered = filtered.filter(item => item.category === catSelect);
    }
    if (query) {
        filtered = filtered.filter(item =>
            item.name.toLowerCase().includes(query) ||
            item.description.toLowerCase().includes(query)
        );
    }

    if (filtered.length === 0) {
        menuGrid.innerHTML = `<p style="grid-column:1/-1; text-align:center; padding: 40px 0;">No matching kitchen menu items.</p>`;
        return;
    }

    menuGrid.innerHTML = filtered.map(item => {
        const isFav = appState.favorites.includes(item.id) ? 'active' : '';
        const heartIcon = isFav ? 'fa-solid fa-heart' : 'fa-regular fa-heart';
        const soldOutClass = item.is_sold_out ? 'sold-out' : '';
        const starHTML = generateStarsHTML(item.rating);
        const itemImage = getMenuItemImage(item);

        return `
            <div class="menu-card glass-card ${soldOutClass}">
                ${item.is_sold_out ? '<div class="sold-out-overlay">Sold Out</div>' : ''}
                <div class="menu-card-img-wrap">
                    <span class="menu-card-category">${item.category}</span>
                    <img src="${itemImage}" class="menu-card-img" alt="${item.name}">
                    <div class="btn-favorite ${isFav}" onclick="toggleFavorite(${item.id})" id="fav-btn-${item.id}">
                        <i class="${heartIcon}"></i>
                    </div>
                </div>
                <div class="menu-card-body">
                    <h3 class="menu-card-title">${item.name}</h3>
                    <p class="menu-card-desc">${item.description}</p>
                    <div class="menu-card-meta">
                        <div class="menu-card-rating">
                            ${starHTML}
                            <span class="menu-card-reviews">(${item.reviews_count} reviews)</span>
                        </div>
                    </div>
                    <div class="menu-card-meta" style="margin-bottom:0;">
                        <span class="menu-card-price">${formatCurrency(item.price)}</span>
                        <div class="menu-card-actions">
                            <button class="btn-add-cart" onclick="addToCart(${item.id})">Add to Cart</button>
                            <button class="btn-review-modal" onclick="openReviewModal(${item.id}, '${item.name.replace(/'/g, "\\'")}')" title="Write Review">
                                <i class="fa-regular fa-comment"></i>
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

function filterCategoryCust() {
    renderCustomerMenu();
}

function searchMenuCust() {
    renderCustomerMenu();
}

// Load Favorites IDs
async function loadFavoritesList() {
    try {
        const favs = await makeRequest(`get_favorites&customerId=${appState.user.id}`);
        appState.favorites = favs;

        // Refresh menus if currently showing favorites
        if (appState.activeTab === 'favorites') {
            renderFavoritesList();
        } else {
            renderCustomerMenu();
        }
    } catch (e) {
        console.warn('Failed to sync favorites checklist.');
    }
}

async function toggleFavorite(itemId) {
    try {
        const response = await makeRequest('toggle_favorite', 'POST', {
            customerId: appState.user.id.toString(),
            menuItemId: itemId.toString()
        });

        if (response.status === 'success') {
            if (response.state === 'added') {
                appState.favorites.push(itemId);
            } else {
                appState.favorites = appState.favorites.filter(id => id !== itemId);
            }

            // Sync heart icons in UI
            const btn = document.getElementById(`fav-btn-${itemId}`);
            if (btn) {
                if (response.state === 'added') {
                    btn.classList.add('active');
                    btn.querySelector('i').className = 'fa-solid fa-heart';
                } else {
                    btn.classList.remove('active');
                    btn.querySelector('i').className = 'fa-regular fa-heart';
                }
            }

            if (appState.activeTab === 'favorites') {
                renderFavoritesList();
            }
        }
    } catch (e) {
        console.error('Error toggling favorite:', e);
    }
}

function renderFavoritesList() {
    const grid = document.getElementById('favorites-grid');
    if (!grid) return;

    const favoriteItems = appState.menu.filter(item => appState.favorites.includes(item.id));
    if (favoriteItems.length === 0) {
        grid.innerHTML = `<p style="grid-column:1/-1; text-align:center; color: var(--text-secondary); padding: 40px 0;">You haven't favorited any dishes yet.</p>`;
        return;
    }

    grid.innerHTML = favoriteItems.map(item => {
        const soldOutClass = item.is_sold_out ? 'sold-out' : '';
        const starHTML = generateStarsHTML(item.rating);
        const itemImage = getMenuItemImage(item);

        return `
            <div class="menu-card glass-card ${soldOutClass}">
                ${item.is_sold_out ? '<div class="sold-out-overlay">Sold Out</div>' : ''}
                <div class="menu-card-img-wrap">
                    <span class="menu-card-category">${item.category}</span>
                    <img src="${itemImage}" class="menu-card-img" alt="${item.name}">
                    <div class="btn-favorite active" onclick="toggleFavorite(${item.id})">
                        <i class="fa-solid fa-heart"></i>
                    </div>
                </div>
                <div class="menu-card-body">
                    <h3 class="menu-card-title">${item.name}</h3>
                    <p class="menu-card-desc">${item.description}</p>
                    <div class="menu-card-meta">
                        <div class="menu-card-rating">
                            ${starHTML}
                            <span class="menu-card-reviews">(${item.reviews_count} reviews)</span>
                        </div>
                    </div>
                    <div class="menu-card-meta" style="margin-bottom:0;">
                        <span class="menu-card-price">${formatCurrency(item.price)}</span>
                        <div class="menu-card-actions">
                            <button class="btn-add-cart" onclick="addToCart(${item.id})">Add to Cart</button>
                        </div>
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

// Shopping Cart Functions
function getLocalCartKey() {
    return `stc_cart_${appState.user.id}`;
}

function addToCart(itemId) {
    const key = getLocalCartKey();
    let cart = JSON.parse(localStorage.getItem(key) || '[]');
    const existing = cart.find(i => i.id === itemId);

    if (existing) {
        existing.qty += 1;
    } else {
        cart.push({ id: itemId, qty: 1 });
    }

    localStorage.setItem(key, JSON.stringify(cart));
    updateCartSidebarCount();
    alert('Item added to cart!');
}

function updateCartSidebarCount() {
    const countSpan = document.getElementById('sidebar-cart-count');
    if (!countSpan) return;

    const key = getLocalCartKey();
    const cart = JSON.parse(localStorage.getItem(key) || '[]');
    const totalQty = cart.reduce((sum, item) => sum + item.qty, 0);
    countSpan.innerText = totalQty;
}

// Render Cart View
function renderCartView() {
    const container = document.getElementById('cart-items-list');
    if (!container) return;

    const key = getLocalCartKey();
    const cart = JSON.parse(localStorage.getItem(key) || '[]');

    if (cart.length === 0) {
        container.innerHTML = `<p style="text-align: center; color: var(--text-secondary); padding: 40px 0;">Your shopping cart is currently empty. Go add some delicious food!</p>`;
        updateCartTotals(0);
        return;
    }

    let subtotal = 0;

    container.innerHTML = cart.map(cartItem => {
        const itemInfo = appState.menu.find(m => m.id === cartItem.id);
        if (!itemInfo) return '';

        const lineTotal = itemInfo.price * cartItem.qty;
        subtotal += lineTotal;

        return `
            <div class="cart-item glass-card">
                <div class="cart-item-details">
                    <div class="cart-item-icon">
                        <i class="${getCategoryIconClass(itemInfo.category)}"></i>
                    </div>
                    <div>
                        <div class="cart-item-name">${itemInfo.name}</div>
                        <div class="cart-item-price">${formatCurrency(itemInfo.price)}</div>
                    </div>
                </div>
                <div class="cart-item-qty-control">
                    <div class="btn-qty" onclick="changeCartQty(${cartItem.id}, -1)"><i class="fa-solid fa-minus"></i></div>
                    <span style="font-weight:600; font-size:14px; min-width:16px; text-align:center;">${cartItem.qty}</span>
                    <div class="btn-qty" onclick="changeCartQty(${cartItem.id}, 1)"><i class="fa-solid fa-plus"></i></div>
                </div>
                <div style="display:flex; align-items:center; gap: 20px;">
                    <div class="cart-item-total">${formatCurrency(lineTotal)}</div>
                    <div class="btn-remove-cart" onclick="removeCartItem(${cartItem.id})"><i class="fa-regular fa-trash-can"></i></div>
                </div>
            </div>
        `;
    }).join('');

    updateCartTotals(subtotal);
    updateCartSidebarCount();
}

function changeCartQty(itemId, change) {
    const key = getLocalCartKey();
    let cart = JSON.parse(localStorage.getItem(key) || '[]');
    const index = cart.findIndex(i => i.id === itemId);

    if (index !== -1) {
        cart[index].qty += change;
        if (cart[index].qty <= 0) {
            cart.splice(index, 1);
        }
        localStorage.setItem(key, JSON.stringify(cart));
        renderCartView();
    }
}

function removeCartItem(itemId) {
    const key = getLocalCartKey();
    let cart = JSON.parse(localStorage.getItem(key) || '[]');
    cart = cart.filter(i => i.id !== itemId);
    localStorage.setItem(key, JSON.stringify(cart));
    renderCartView();
}

function updateCartTotals(subtotal) {
    const subtotalSpan = document.getElementById('cart-subtotal');
    const discountSpan = document.getElementById('cart-discount');
    const totalSpan = document.getElementById('cart-total');

    if (!subtotalSpan) return;

    let discount = subtotal * appState.couponDiscount;
    let total = subtotal - discount;
    if (total < 0) total = 0;

    subtotalSpan.innerText = formatCurrency(subtotal);
    discountSpan.innerText = `-${formatCurrency(discount)} (${(appState.couponDiscount * 100).toFixed(0)}%)`;
    totalSpan.innerText = formatCurrency(total);
}

async function applyCouponCode() {
    const code = document.getElementById('coupon-code-input').value.trim().toUpperCase();
    if (!code) {
        appState.couponDiscount = 0.0;
        appState.appliedCoupon = '';
        renderCartView();
        return;
    }

    try {
        const coupons = await makeRequest('get_coupons');
        const match = coupons.find(c => c.code.toUpperCase() === code && (c.is_active === 1 || c.is_active === true));

        if (match) {
            appState.couponDiscount = parseFloat(match.discount_percent) / 100.0;
            appState.appliedCoupon = code;
            alert(`Coupon applied! ${match.discount_percent}% off your cart.`);
            renderCartView();
        } else {
            alert('Invalid or expired coupon code.');
            appState.couponDiscount = 0.0;
            appState.appliedCoupon = '';
            renderCartView();
        }
    } catch (e) {
        alert('Failed to validate discount code.');
    }
}

function selectPaymentMethod(method) {
    appState.selectedPaymentMethod = method;

    // Toggle active classes
    const optWallet = document.getElementById('pay-opt-wallet');
    const optCOD = document.getElementById('pay-opt-cod');

    if (method === 'Wallet') {
        optWallet.classList.add('active');
        optWallet.querySelector('input').checked = true;
        optCOD.classList.remove('active');
    } else {
        optCOD.classList.add('active');
        optCOD.querySelector('input').checked = true;
        optWallet.classList.remove('active');
    }
}

async function placeOrder() {
    const key = getLocalCartKey();
    const cart = JSON.parse(localStorage.getItem(key) || '[]');

    if (cart.length === 0) {
        alert('Your shopping cart is empty!');
        return;
    }

    // Double check wallet balance if Wallet option chosen
    let subtotal = 0;
    const itemsList = cart.map(cartItem => {
        const itemInfo = appState.menu.find(m => m.id === cartItem.id);
        if (itemInfo) subtotal += itemInfo.price * cartItem.qty;
        return `${cartItem.id}:${cartItem.qty}`;
    }).join(',');

    let total = subtotal - (subtotal * appState.couponDiscount);
    if (total < 0) total = 0;

    if (appState.selectedPaymentMethod === 'Wallet') {
        const myBalance = parseFloat(appState.user.wallet.balance);
        if (myBalance < total) {
            alert(`Insufficient funds in STC Pay. Your order totals ${formatCurrency(total)}, but you only have ${formatCurrency(myBalance)}. Please load wallet balance first.`);
            openWalletModal();
            return;
        }
    }

    try {
        const response = await makeRequest('place_order', 'POST', {
            customerId: appState.user.id.toString(),
            items: itemsList,
            totalAmount: total.toFixed(2),
            paymentMethod: appState.selectedPaymentMethod
        });

        if (response.status === 'success') {
            alert(`Order #${response.orderId} placed successfully!`);
            // Clear cart
            localStorage.setItem(key, '[]');
            appState.couponDiscount = 0.0;
            appState.appliedCoupon = '';
            document.getElementById('coupon-code-input').value = '';

            // Set active tracking order ID
            appState.currentOrder = { id: response.orderId };
            localStorage.setItem(`stc_active_order_${appState.user.id}`, response.orderId);

            // Sync balance and switch
            await syncWalletBalance();
            switchDashboardTab('tracker');
        } else {
            alert(response.message || 'Could not checkout.');
        }
    } catch (e) {
        alert(e.message || 'Transaction failed. Check connection.');
    }
}

// Live Stepper Order Tracker
async function pollActiveOrderTracker() {
    if (!appState.user) return;

    const activeOrderId = localStorage.getItem(`stc_active_order_${appState.user.id}`);
    if (!activeOrderId) return;

    try {
        const order = await makeRequest(`get_order&orderId=${activeOrderId}`);
        if (order.status === 'error') {
            localStorage.removeItem(`stc_active_order_${appState.user.id}`);
            appState.currentOrder = null;
            if (appState.activeTab === 'tracker') renderLiveTracker();
            return;
        }

        appState.currentOrder = order;

        if (appState.activeTab === 'tracker') {
            renderLiveTracker();
        }

        // Check if completed to remove tracker loop
        if (order.status === 'Completed') {
            localStorage.removeItem(`stc_active_order_${appState.user.id}`);
            // Play a success sound or alert
            setTimeout(() => {
                alert(`Order #${order.orderId} has been delivered! Enjoy your meal!`);
            }, 1000);
        }

        // Sync notifications too
        fetchNotificationsList();

    } catch (e) {
        console.warn('Tracker poll failed.');
    }
}

function renderLiveTracker() {
    const container = document.getElementById('tracker-container');
    if (!container) return;

    if (!appState.currentOrder) {
        const savedOrderId = localStorage.getItem(`stc_active_order_${appState.user.id}`);
        if (savedOrderId) {
            // Initiate load
            container.innerHTML = `
                <div class="glass-card" style="padding:40px; text-align:center;">
                    <i class="fa-solid fa-spinner fa-spin" style="font-size:40px; color:var(--stc-purple);"></i>
                    <p style="margin-top:16px;">Connecting to Live Kitchen Stream...</p>
                </div>
            `;
            pollActiveOrderTracker();
            return;
        }

        container.innerHTML = `
            <div class="glass-card" style="padding: 40px; text-align: center;">
                <i class="fa-solid fa-truck-fast" style="font-size: 50px; color: var(--border-color); margin-bottom: 16px;"></i>
                <p style="color: var(--text-secondary);">You have no active orders. Place an order to see live tracking!</p>
            </div>
        `;
        return;
    }

    const o = appState.currentOrder;
    let progressPct = 0;
    let step1 = '', step2 = '', step3 = '', step4 = '';

    if (o.status === 'Order Received') {
        progressPct = 0;
        step1 = 'active';
    } else if (o.status === 'Preparing') {
        progressPct = 33;
        step1 = 'completed';
        step2 = 'active';
    } else if (o.status === 'Ready') {
        progressPct = 66;
        step1 = 'completed';
        step2 = 'completed';
        step3 = 'active';
    } else if (o.status === 'Completed') {
        progressPct = 100;
        step1 = 'completed';
        step2 = 'completed';
        step3 = 'completed';
        step4 = 'completed';
    }

    container.innerHTML = `
        <div class="tracker-box glass-card">
            <div class="tracker-header">
                <span class="tracker-id">Active Order #${o.orderId}</span>
                <span class="tracker-eta"><i class="fa-regular fa-clock"></i> ETA: ${o.preparation_time} Mins remaining</span>
            </div>
            
            <div class="stepper">
                <div class="stepper-progress" style="width: ${progressPct}%;"></div>
                
                <div class="step ${step1}">
                    <div class="step-icon"><i class="fa-solid fa-receipt"></i></div>
                    <span class="step-label">Received</span>
                </div>
                <div class="step ${step2}">
                    <div class="step-icon"><i class="fa-solid fa-fire-burner"></i></div>
                    <span class="step-label">Preparing</span>
                </div>
                <div class="step ${step3}">
                    <div class="step-icon"><i class="fa-solid fa-bag-shopping"></i></div>
                    <span class="step-label">Ready</span>
                </div>
                <div class="step ${step4}">
                    <div class="step-icon"><i class="fa-solid fa-circle-check"></i></div>
                    <span class="step-label">Completed</span>
                </div>
            </div>

            <div style="margin-top: 24px; padding-top: 20px; border-top:1px solid var(--border-color); font-size:14px;">
                <div style="display:flex; justify-content:space-between; margin-bottom:8px;">
                    <span style="color:var(--text-secondary);">Payment Method:</span>
                    <span style="font-weight:600;">${o.payment_method}</span>
                </div>
                <div style="display:flex; justify-content:space-between; margin-bottom:8px;">
                    <span style="color:var(--text-secondary);">Checkout items:</span>
                    <span style="font-weight:500;">${o.items}</span>
                </div>
                <div style="display:flex; justify-content:space-between;">
                    <span style="color:var(--text-secondary);">Total billing amount:</span>
                    <span style="font-weight:700; color:var(--stc-purple); font-size:16px;">${formatCurrency(o.total_amount)}</span>
                </div>
            </div>
        </div>
    `;
}

// Notification Inbox Management
async function fetchNotificationsList() {
    if (!appState.user) return;

    const container = document.getElementById('notifications-list');
    const badge = document.getElementById('sidebar-inbox-unread-count');

    try {
        const list = await makeRequest(`get_notifications&customerId=${appState.user.id}`);
        const unreadCount = list.filter(n => n.is_read === 0 || n.is_read === false).length;

        if (badge) {
            if (unreadCount > 0) {
                badge.innerText = unreadCount;
                badge.style.display = 'inline-block';
            } else {
                badge.style.display = 'none';
            }
        }

        if (!container) return;

        if (list.length === 0) {
            container.innerHTML = `<p style="text-align:center; color:var(--text-secondary); padding: 30px 0;">No system alerts inside inbox.</p>`;
            return;
        }

        container.innerHTML = list.map(n => {
            const isUnread = (n.is_read === 0 || n.is_read === false) ? 'unread' : '';
            return `
                <div class="notification-item glass-card ${isUnread}">
                    <div class="notification-item-icon">
                        <i class="fa-solid fa-bell"></i>
                    </div>
                    <div class="notification-body">
                        <div class="notification-text">${n.message}</div>
                        <div class="notification-time">${n.timestamp}</div>
                    </div>
                </div>
            `;
        }).join('');

    } catch (e) {
        console.warn('Failed to fetch notifications.');
    }
}

async function markAllNotificationsRead() {
    try {
        const response = await makeRequest('mark_notifications_read', 'POST', { customerId: appState.user.id.toString() });
        if (response.status === 'success') {
            fetchNotificationsList();
        }
    } catch (e) {
        console.error('Error marking notifications read:', e);
    }
}

// Order History
async function loadCustomerOrderHistory() {
    const tbody = document.getElementById('history-table-body');
    if (!tbody) return;

    try {
        const orders = await makeRequest(`get_customer_orders&customerId=${appState.user.id}`);
        if (orders.length === 0) {
            tbody.innerHTML = `<tr><td colspan="7" class="text-center" style="color:var(--text-secondary); padding:30px;">You haven't made any purchases yet.</td></tr>`;
            return;
        }

        tbody.innerHTML = orders.map(o => {
            let statusBadge = '';
            if (o.status === 'Order Received') statusBadge = '<span class="badge received">Received</span>';
            else if (o.status === 'Preparing') statusBadge = '<span class="badge preparing">Preparing</span>';
            else if (o.status === 'Ready') statusBadge = '<span class="badge ready">Ready</span>';
            else if (o.status === 'Completed') statusBadge = '<span class="badge completed">Completed</span>';

            return `
                <tr>
                    <td style="font-weight:700;">#${o.orderId}</td>
                    <td style="font-size:12px; color:var(--text-secondary);">${o.timestamp}</td>
                    <td style="max-width:250px; overflow:hidden; text-overflow:ellipsis; white-space:nowrap;" title="${o.items}">${o.items}</td>
                    <td style="font-weight:600;">${formatCurrency(o.total_amount)}</td>
                    <td style="font-size:12px;">${o.payment_method}</td>
                    <td>${statusBadge}</td>
                    <td>
                        <button class="btn-submit" style="padding: 6px 12px; font-size:12px; margin:0;" onclick="openReviewModalFromHistory('${o.items}')">Review Item</button>
                    </td>
                </tr>
            `;
        }).join('');
    } catch (e) {
        tbody.innerHTML = `<tr><td colspan="7" class="text-center">Failed to fetch order archive.</td></tr>`;
    }
}

function openReviewModalFromHistory(itemsString) {
    // Parse first item from order string "itemId:qty,itemId:qty"
    const firstPair = itemsString.split(',')[0];
    if (firstPair) {
        const itemId = parseInt(firstPair.split(':')[0]);
        const matchedItem = appState.menu.find(m => m.id === itemId);
        if (matchedItem) {
            openReviewModal(itemId, matchedItem.name);
        }
    }
}

// Wallet Modals & Logic
function openWalletModal() {
    document.getElementById('wallet-modal').classList.add('active');
    setWalletMethod('CreditCard');
}

function setWalletMethod(method) {
    appState.walletMethod = method;
    const btnCard = document.getElementById('wallet-btn-card');
    const btnQR = document.getElementById('wallet-btn-qr');
    const cardForm = document.getElementById('wallet-card-form');
    const qrPane = document.getElementById('wallet-qr-pane');

    if (method === 'CreditCard') {
        btnCard.className = 'btn-submit';
        btnCard.style.background = 'var(--stc-purple)';
        btnQR.className = 'btn-submit';
        btnQR.style.background = 'var(--bg-primary)';
        btnQR.style.color = 'var(--text-primary)';
        btnQR.style.border = '1px solid var(--border-color)';

        cardForm.style.display = 'block';
        qrPane.style.display = 'none';
        document.getElementById('wallet-load-method').value = 'CreditCard';
    } else {
        btnQR.className = 'btn-submit';
        btnQR.style.background = 'var(--stc-purple)';
        btnCard.className = 'btn-submit';
        btnCard.style.background = 'var(--bg-primary)';
        btnCard.style.color = 'var(--text-primary)';
        btnCard.style.border = '1px solid var(--border-color)';

        cardForm.style.display = 'none';
        qrPane.style.display = 'block';
        document.getElementById('wallet-load-method').value = 'QR';
    }
}

async function handleWalletLoad(event) {
    event.preventDefault();
    const amount = parseFloat(document.getElementById('wallet-amount-card').value);

    if (amount <= 0 || isNaN(amount)) {
        alert('Please enter a valid deposit amount.');
        return;
    }

    try {
        const response = await makeRequest('add_wallet_balance', 'POST', {
            customerId: appState.user.id.toString(),
            amount: amount.toFixed(2),
            method: 'CreditCard'
        });

        if (response.status === 'success') {
            alert(`PKR ${amount.toLocaleString()} loaded successfully to your E-Wallet!`);
            closeModal('wallet-modal');
            syncWalletBalance();
            fetchNotificationsList();
        } else {
            alert(response.message || 'Deposit transaction declined.');
        }
    } catch (e) {
        alert('Deposit transaction failed.');
    }
}

async function triggerQRDeposit() {
    const amountVal = parseFloat(document.getElementById('wallet-amount-qr').value);

    if (amountVal <= 0 || isNaN(amountVal)) {
        alert('Please enter a valid deposit amount first.');
        return;
    }

    try {
        const response = await makeRequest('add_wallet_balance', 'POST', {
            customerId: appState.user.id.toString(),
            amount: amountVal.toFixed(2),
            method: 'QR Code App'
        });

        if (response.status === 'success') {
            alert(`QR authorized successfully. Loaded PKR ${amountVal.toLocaleString()} into STC Wallet!`);
            closeModal('wallet-modal');
            document.getElementById('wallet-amount-qr').value = '';
            syncWalletBalance();
            fetchNotificationsList();
        } else {
            alert('Failed to authorize QR transaction.');
        }
    } catch (e) {
        alert('QR API gateway connection timed out.');
    }
}

// Review Portal
function openReviewModal(itemId, itemName) {
    document.getElementById('review-item-id').value = itemId;
    document.getElementById('review-item-name').innerText = `Write a Review for: ${itemName}`;
    document.getElementById('review-comment').value = '';
    setStarRating(5);
    document.getElementById('review-modal').classList.add('active');
}

function setStarRating(stars) {
    appState.starRating = stars;
    document.getElementById('review-rating-value').value = stars;

    const starIcons = document.querySelectorAll('#rating-star-selector i');
    starIcons.forEach(icon => {
        const val = parseInt(icon.getAttribute('data-value'));
        if (val <= stars) {
            icon.className = 'fa-solid fa-star active';
        } else {
            icon.className = 'fa-solid fa-star';
        }
    });
}

async function handleReviewSubmit(event) {
    event.preventDefault();
    const itemId = document.getElementById('review-item-id').value;
    const rating = document.getElementById('review-rating-value').value;
    const comment = document.getElementById('review-comment').value.trim();

    try {
        const response = await makeRequest('add_review', 'POST', {
            customerId: appState.user.id.toString(),
            menuItemId: itemId,
            rating: rating,
            comment: comment
        });

        if (response.status === 'success') {
            alert('Thank you! Your feedback has been registered.');
            closeModal('review-modal');
            loadCustomerMenu(); // Refresh counts
        } else {
            alert(response.message || 'Failed to submit review.');
        }
    } catch (e) {
        alert('Error writing review.');
    }
}

// =================================================================
// 7. FLOATING SUPPORT CHAT WIDGET
// =================================================================
function toggleChatWindow() {
    appState.chatOpen = !appState.chatOpen;
    const win = document.getElementById('support-chat-window');
    if (win) {
        if (appState.chatOpen) win.classList.add('active');
        else win.classList.remove('active');
    }
}

function handleChatKeyPress(event) {
    if (event.key === 'Enter') {
        sendSupportMessage();
    }
}

function sendSupportMessage() {
    const input = document.getElementById('chat-text-input');
    const container = document.getElementById('chat-messages-container');
    if (!input || !input.value.trim() || !container) return;

    const userText = input.value.trim();
    input.value = '';

    // Append user message
    container.innerHTML += `
        <div class="chat-msg customer">${userText}</div>
    `;
    container.scrollTop = container.scrollHeight;

    // Simulate smart bot response based on keywords
    let responseText = "I'm sorry, I'm just a simple C++ CGI simulated support agent. Can you specify if you need help with 'wallet', 'order', 'menu', or 'refund'?";
    const textLow = userText.toLowerCase();

    if (textLow.includes('wallet') || textLow.includes('balance') || textLow.includes('money')) {
        responseText = "Ahlan! You can load your wallet anytime by clicking 'Add Money' in the sidebar. You can use mock Credit Cards or QR codes.";
    } else if (textLow.includes('order') || textLow.includes('track') || textLow.includes('eta')) {
        responseText = "Check your 'Live Order Tracker' tab! It lists all current prep milestones directly from the kitchen database.";
    } else if (textLow.includes('menu') || textLow.includes('burger') || textLow.includes('pizza')) {
        responseText = "Our signature dish is the STC Signature Burger! It features a premium beef patty and our delicious orange sauce.";
    } else if (textLow.includes('admin') || textLow.includes('policy')) {
        responseText = "For security policies, all admin alterations log directly inside the C++ flat text files.";
    } else if (textLow.includes('hi') || textLow.includes('hello') || textLow.includes('ahlan')) {
        responseText = "Ahlan! How can I assist you with your STC Express Dining experience today?";
    }

    setTimeout(() => {
        container.innerHTML += `
            <div class="chat-msg support">${responseText}</div>
        `;
        container.scrollTop = container.scrollHeight;
    }, 800);
}

// =================================================================
// 8. ADMIN DASHBOARD OPERATIONS
// =================================================================
function setupAdminDashboard() {
    // Fill sidebar basic
    document.getElementById('nav-user-avatar').innerText = 'A';
    document.getElementById('nav-username').innerText = 'Admin';

    // Kick loaders
    loadAdminAnalytics();
    loadAdminOrders();
    loadAdminMenu();
    loadAdminCoupons();
    loadAdminAnnouncementsList();
    loadAdminCustomers();
    loadAdminNotifications();
    
    // Setup interval to auto-fetch new orders and notifications for admin
    setInterval(() => {
        if (appState.activeAdminTab === 'orders') loadAdminOrders();
        loadAdminNotifications();
    }, 5000);
}

function switchAdminTab(tabId) {
    appState.activeAdminTab = tabId;

    // Sidebar active styles
    const tabBtns = document.querySelectorAll('.sidebar-menu-item');
    tabBtns.forEach(btn => {
        if (btn.id === `tab-btn-${tabId}`) {
            btn.classList.add('active');
        } else {
            btn.classList.remove('active');
        }
    });

    // Content Views toggles
    const views = document.querySelectorAll('.tab-view');
    views.forEach(view => {
        if (view.id === `view-${tabId}`) {
            view.classList.add('active');
        } else {
            view.classList.remove('active');
        }
    });

    // Specific loaders
    if (tabId === 'analytics') loadAdminAnalytics();
    else if (tabId === 'notifications') loadAdminNotifications();
    else if (tabId === 'orders') loadAdminOrders();
    else if (tabId === 'menu') loadAdminMenu();
    else if (tabId === 'coupons') loadAdminCoupons();
    else if (tabId === 'announcements') loadAdminAnnouncementsList();
    else if (tabId === 'customers') loadAdminCustomers();
}

// Analytics Tab
async function loadAdminAnalytics() {
    try {
        const stats = await makeRequest('get_sales_analytics');

        // Update stats top counters
        document.getElementById('stats-revenue').innerText = formatCurrency(stats.total_revenue);
        document.getElementById('stats-orders').innerText = stats.total_orders;
        document.getElementById('stats-customers').innerText = stats.total_customers;

        // Render categories breakdown
        const catsBox = document.getElementById('stats-categories-container');
        if (catsBox) {
            if (stats.category_revenue.length === 0) {
                catsBox.innerHTML = '<p style="color:var(--text-secondary); font-size:13px;">No category revenue tracked yet.</p>';
            } else {
                catsBox.innerHTML = stats.category_revenue.map(c => `
                    <div>
                        <div style="display:flex; justify-content:space-between; font-size:13px; font-weight:600; margin-bottom:4px;">
                            <span>${c.category}</span>
                            <span>${formatCurrency(c.revenue)}</span>
                        </div>
                        <div style="width:100%; height:8px; background:var(--border-color); border-radius:4px; overflow:hidden;">
                            <div style="height:100%; background:var(--stc-purple); width:${Math.min((c.revenue / Math.max(stats.total_revenue, 1)) * 100, 100)}%;"></div>
                        </div>
                    </div>
                `).join('');
            }
        }

        // Render Recent Activity list
        const activityBox = document.getElementById('stats-activity-container');
        if (activityBox) {
            if (stats.daily_orders.length === 0) {
                activityBox.innerHTML = '<p style="color:var(--text-secondary); font-size:13px;">No transaction logs.</p>';
            } else {
                activityBox.innerHTML = stats.daily_orders.map(d => `
                    <div style="display:flex; justify-content:space-between; align-items:center; padding:10px; border-bottom:1px solid var(--border-color); font-size:13px;">
                        <span><i class="fa-regular fa-calendar-check" style="color:var(--stc-purple); margin-right:8px;"></i> ${d.date}</span>
                        <span style="font-weight:600;">${d.orders_count} Checkouts</span>
                    </div>
                `).join('');
            }
        }

    } catch (e) {
        console.warn('Analytics loading failed.');
    }
}

// Admin Notifications Tab
async function loadAdminNotifications() {
    const listContainer = document.getElementById('admin-notifications-list');
    const badge = document.getElementById('admin-notif-badge');
    if (!listContainer) return;

    try {
        // Admin ID is conventionally 1
        const notifs = await makeRequest(`get_notifications&customerId=1`);
        const unreadCount = notifs.filter(n => n.is_read === 0).length;

        // Update Badge
        if (badge) {
            badge.innerText = unreadCount;
            badge.style.display = unreadCount > 0 ? 'inline-block' : 'none';
        }

        if (notifs.length === 0) {
            listContainer.innerHTML = '<div class="glass-card" style="padding:24px; text-align:center; color:var(--text-secondary);">No new order notifications.</div>';
            return;
        }

        listContainer.innerHTML = notifs.map(n => {
            const bg = n.is_read ? 'var(--bg-primary)' : '#fff3cd'; // Highlight unread
            const border = n.is_read ? '1px solid var(--border-color)' : '1px solid #ffeeba';
            const icon = n.is_read ? '<i class="fa-solid fa-bell" style="color:var(--text-secondary);"></i>' : '<i class="fa-solid fa-bell-ringing" style="color:#856404;"></i>';
            const textCol = n.is_read ? 'var(--text-secondary)' : '#856404';

            return `
                <div class="glass-card" style="padding:16px; background:${bg}; border:${border}; display:flex; align-items:flex-start; gap:16px;">
                    <div style="margin-top:2px;">${icon}</div>
                    <div style="flex: 1;">
                        <div style="font-size:14px; font-weight:600; color:${textCol}; margin-bottom:4px;">${n.message}</div>
                        <div style="font-size:11px; color:var(--text-secondary);">${n.timestamp}</div>
                    </div>
                </div>
            `;
        }).join('');

    } catch (e) {
        listContainer.innerHTML = '<div class="glass-card" style="padding:24px; text-align:center;">Failed to load notifications.</div>';
    }
}

async function markAdminNotificationsRead() {
    try {
        const response = await makeRequest('mark_notifications_read', 'POST', { customerId: "1" });
        if (response.status === 'success') {
            loadAdminNotifications();
        }
    } catch (e) {
        console.error('Error marking admin notifications read:', e);
    }
}

// Orders Tab
async function loadAdminOrders() {
    const tbody = document.getElementById('admin-orders-table-body');
    if (!tbody) return;

    try {
        const orders = await makeRequest('get_all_orders');
        if (orders.length === 0) {
            tbody.innerHTML = `<tr><td colspan="8" class="text-center" style="color:var(--text-secondary); padding:30px;">No checkout orders recorded in database.</td></tr>`;
            return;
        }

        tbody.innerHTML = orders.map(o => {
            let statusClass = '';
            if (o.status === 'Order Received') statusClass = 'received';
            else if (o.status === 'Preparing') statusClass = 'preparing';
            else if (o.status === 'Ready') statusClass = 'ready';
            else if (o.status === 'Completed') statusClass = 'completed';

            return `
                <tr>
                    <td style="font-weight:700;">#${o.orderId}</td>
                    <td>Customer #${o.customerId}</td>
                    <td style="max-width:200px; overflow:hidden; text-overflow:ellipsis; white-space:nowrap;" title="${o.items}">${o.items}</td>
                    <td style="font-weight:600;">${formatCurrency(o.total_amount)}</td>
                    <td style="font-size:12px; color:var(--text-secondary);">${o.timestamp}</td>
                    <td>
                        <select onchange="updateOrderStatus(${o.orderId}, this.value)" style="padding: 6px 12px; border-radius:20px; font-size:12px; font-weight:600; background: var(--bg-primary); border: 1px solid var(--border-color); color:var(--text-primary);">
                            <option value="Order Received" ${o.status === 'Order Received' ? 'selected' : ''}>Order Received</option>
                            <option value="Preparing" ${o.status === 'Preparing' ? 'selected' : ''}>Preparing</option>
                            <option value="Ready" ${o.status === 'Ready' ? 'selected' : ''}>Ready</option>
                            <option value="Completed" ${o.status === 'Completed' ? 'selected' : ''}>Completed</option>
                        </select>
                    </td>
                    <td>
                        <input type="number" value="${o.preparation_time}" style="width:60px; padding:6px; border-radius:var(--radius-sm); border:1px solid var(--border-color); background:var(--bg-primary); color:var(--text-primary); text-align:center;" onchange="updateOrderPrepTime(${o.orderId}, this.value)">
                        <span style="font-size:11px; color:var(--text-secondary);"> mins</span>
                    </td>
                    <td>
                        <span class="badge ${statusClass}" style="margin-bottom:8px; display:inline-block;">${o.status}</span>
                        <div class="btn-action" style="padding: 4px 8px; font-size: 11px; background: var(--stc-purple); color: white; border-radius: 4px; cursor: pointer; text-align: center;" onclick="openAdminTrackerModal(${o.orderId})" title="View Live Tracking"><i class="fa-solid fa-route"></i> Live Track</div>
                    </td>
                </tr>
            `;
        }).join('');
    } catch (e) {
        tbody.innerHTML = `<tr><td colspan="8" class="text-center">Failed to fetch order streams.</td></tr>`;
    }
}

async function updateOrderStatus(orderId, newStatus) {
    try {
        const response = await makeRequest('update_order_status', 'POST', {
            orderId: orderId.toString(),
            status: newStatus
        });
        if (response.status === 'success') {
            alert(`Order #${orderId} status set to: ${newStatus}`);
            loadAdminOrders();
        }
    } catch (e) {
        alert('Failed to update status.');
    }
}

async function updateOrderPrepTime(orderId, mins) {
    const minsInt = parseInt(mins);
    if (minsInt < 0 || isNaN(minsInt)) return;

    try {
        const response = await makeRequest('update_prep_time', 'POST', {
            orderId: orderId.toString(),
            minutes: minsInt.toString()
        });
        if (response.status === 'success') {
            console.log(`Order #${orderId} ETA updated.`);
        }
    } catch (e) {
        alert('Failed to update preparation time.');
    }
}

async function openAdminTrackerModal(orderId) {
    const modal = document.getElementById('admin-tracker-modal');
    const container = document.getElementById('admin-tracker-preview-container');
    if (!modal || !container) return;
    
    modal.classList.add('active');
    container.innerHTML = '<div style="text-align:center; padding: 20px;"><i class="fa-solid fa-spinner fa-spin" style="font-size:30px; color:var(--stc-purple);"></i></div>';
    
    try {
        const order = await makeRequest(`get_order&orderId=${orderId}`);
        if (order.status === 'error') {
            container.innerHTML = '<p style="text-align:center; color:var(--text-secondary);">Order not found.</p>';
            return;
        }
        
        let progressPct = 0;
        let step1 = '', step2 = '', step3 = '', step4 = '';

        if (order.status === 'Order Received') {
            progressPct = 0;
            step1 = 'active';
        } else if (order.status === 'Preparing') {
            progressPct = 33;
            step1 = 'completed';
            step2 = 'active';
        } else if (order.status === 'Ready') {
            progressPct = 66;
            step1 = 'completed';
            step2 = 'completed';
            step3 = 'active';
        } else if (order.status === 'Completed') {
            progressPct = 100;
            step1 = 'completed';
            step2 = 'completed';
            step3 = 'completed';
            step4 = 'completed';
        }

        container.innerHTML = `
            <div class="tracker-box glass-card" style="box-shadow: none; border: 1px solid var(--border-color);">
                <div class="tracker-header">
                    <span class="tracker-id">Preview: Order #${order.orderId}</span>
                    <span class="tracker-eta"><i class="fa-regular fa-clock"></i> ETA: ${order.preparation_time} Mins</span>
                </div>
                
                <div class="stepper">
                    <div class="stepper-progress" style="width: ${progressPct}%;"></div>
                    
                    <div class="step ${step1}">
                        <div class="step-icon"><i class="fa-solid fa-receipt"></i></div>
                        <span class="step-label">Received</span>
                    </div>
                    <div class="step ${step2}">
                        <div class="step-icon"><i class="fa-solid fa-fire-burner"></i></div>
                        <span class="step-label">Preparing</span>
                    </div>
                    <div class="step ${step3}">
                        <div class="step-icon"><i class="fa-solid fa-bag-shopping"></i></div>
                        <span class="step-label">Ready</span>
                    </div>
                    <div class="step ${step4}">
                        <div class="step-icon"><i class="fa-solid fa-house-chimney"></i></div>
                        <span class="step-label">Completed</span>
                    </div>
                </div>
            </div>
        `;
    } catch (e) {
        container.innerHTML = '<p style="text-align:center;">Failed to load order data.</p>';
    }
}

// Menu Manager Tab
async function loadAdminMenu() {
    const tbody = document.getElementById('admin-menu-table-body');
    if (!tbody) return;

    try {
        const items = await makeRequest('get_menu');
        appState.menu = items;

        if (items.length === 0) {
            tbody.innerHTML = `<tr><td colspan="8" class="text-center" style="color:var(--text-secondary); padding:30px;">No menu items. Add one!</td></tr>`;
            return;
        }

        tbody.innerHTML = items.map(item => {
            return `
                <tr>
                    <td style="font-weight:700;">#${item.id}</td>
                    <td style="font-weight:600;">${item.name}</td>
                    <td style="max-width:250px; overflow:hidden; text-overflow:ellipsis; white-space:nowrap;" title="${item.description}">${item.description}</td>
                    <td>${item.category}</td>
                    <td style="font-weight:600;">${formatCurrency(item.price)}</td>
                    <td>
                        <label class="switch">
                            <input type="checkbox" ${item.is_sold_out ? 'checked' : ''} onchange="toggleMenuItemSoldOut(${item.id}, this.checked)">
                            <span class="slider"></span>
                        </label>
                    </td>
                    <td><i class="fa-solid fa-star" style="color:var(--stc-yellow); margin-right:4px;"></i> ${item.rating}</td>
                    <td>
                        <div class="action-buttons">
                            <div class="btn-action edit" onclick="openMenuItemModal('edit', ${item.id})" title="Edit"><i class="fa-solid fa-pencil"></i></div>
                            <div class="btn-action delete" onclick="deleteMenuItem(${item.id})" title="Delete"><i class="fa-solid fa-trash-can"></i></div>
                        </div>
                    </td>
                </tr>
            `;
        }).join('');
    } catch (e) {
        tbody.innerHTML = `<tr><td colspan="8" class="text-center">Failed to load menu list.</td></tr>`;
    }
}

async function toggleMenuItemSoldOut(id, checked) {
    try {
        const response = await makeRequest('set_sold_out', 'POST', {
            id: id.toString(),
            is_sold_out: checked ? '1' : '0'
        });
        if (response.status === 'success') {
            loadAdminMenu();
        }
    } catch (e) {
        alert('Failed to set sold-out status.');
    }
}

function openMenuItemModal(mode, itemId = null) {
    const modal = document.getElementById('menu-item-modal');
    const title = document.getElementById('menu-modal-title');
    const form = document.getElementById('menu-item-form');
    const soldOutGroup = document.getElementById('menu-item-sold-out-group');

    form.reset();
    document.getElementById('menu-item-id').value = '';

    if (mode === 'add') {
        title.innerText = 'Add Menu Item';
        soldOutGroup.style.display = 'none';
    } else {
        title.innerText = 'Edit Menu Item';
        soldOutGroup.style.display = 'flex';

        // Find item
        const item = appState.menu.find(m => m.id === itemId);
        if (item) {
            document.getElementById('menu-item-id').value = item.id;
            document.getElementById('menu-item-name').value = item.name;
            document.getElementById('menu-item-desc').value = item.description;
            document.getElementById('menu-item-price').value = item.price;
            document.getElementById('menu-item-category').value = item.category;
            document.getElementById('menu-item-sold-out-checkbox').checked = item.is_sold_out;
        }
    }

    modal.classList.add('active');
}

async function handleMenuItemSubmit(event) {
    event.preventDefault();
    const id = document.getElementById('menu-item-id').value;
    const name = document.getElementById('menu-item-name').value.trim();
    const description = document.getElementById('menu-item-desc').value.trim();
    const price = parseFloat(document.getElementById('menu-item-price').value);
    const category = document.getElementById('menu-item-category').value;
    const isSoldOut = document.getElementById('menu-item-sold-out-checkbox').checked;

    if (price <= 0 || isNaN(price)) {
        alert('Price must be greater than zero.');
        return;
    }

    const payload = {
        name,
        description,
        price: price.toFixed(2),
        category
    };

    let action = 'add_menu_item';
    if (id) {
        action = 'update_menu_item';
        payload.id = id;
        payload.is_sold_out = isSoldOut ? '1' : '0';
    }

    try {
        const response = await makeRequest(action, 'POST', payload);
        if (response.status === 'success') {
            alert(response.message || 'Menu item configuration saved.');
            closeModal('menu-item-modal');
            loadAdminMenu();
        } else {
            alert(response.message || 'Action failed.');
        }
    } catch (e) {
        alert('Failed to save menu item changes.');
    }
}

async function deleteMenuItem(id) {
    if (!confirm('Are you sure you want to delete this item?')) return;

    try {
        const response = await makeRequest('delete_menu_item', 'POST', { id: id.toString() });
        if (response.status === 'success') {
            alert('Item removed from kitchen menu.');
            loadAdminMenu();
        }
    } catch (e) {
        alert('Failed to delete item.');
    }
}

// Coupons Tab
async function loadAdminCoupons() {
    const tbody = document.getElementById('admin-coupons-table-body');
    if (!tbody) return;

    try {
        const coupons = await makeRequest('get_coupons');
        if (coupons.length === 0) {
            tbody.innerHTML = `<tr><td colspan="3" class="text-center" style="color:var(--text-secondary); padding:20px;">No coupon codes registered.</td></tr>`;
            return;
        }

        tbody.innerHTML = coupons.map(c => {
            const labelClass = (c.is_active === 1 || c.is_active === true) ? 'active-coupon' : 'inactive-coupon';
            const labelText = (c.is_active === 1 || c.is_active === true) ? 'Active' : 'Expired';

            return `
                <tr>
                    <td style="font-weight:700; letter-spacing:0.5px;">${c.code}</td>
                    <td style="font-weight:600; color:var(--stc-purple);">${c.discount_percent}% Discount</td>
                    <td>
                        <label class="switch">
                            <input type="checkbox" ${(c.is_active === 1 || c.is_active === true) ? 'checked' : ''} onchange="toggleCouponActive('${c.code}', this.checked)">
                            <span class="slider"></span>
                        </label>
                        <span class="badge ${labelClass}" style="margin-left: 12px;">${labelText}</span>
                    </td>
                </tr>
            `;
        }).join('');
    } catch (e) {
        tbody.innerHTML = `<tr><td colspan="3" class="text-center">Failed to load coupons.</td></tr>`;
    }
}

function openCouponModal() {
    document.getElementById('coupon-form').reset();
    document.getElementById('coupon-modal').classList.add('active');
}

async function handleCouponSubmit(event) {
    event.preventDefault();
    const code = document.getElementById('coupon-code').value.trim().toUpperCase();
    const discount = parseFloat(document.getElementById('coupon-discount').value);

    if (discount <= 0 || discount > 100 || isNaN(discount)) {
        alert('Discount percent must be between 1 and 100.');
        return;
    }

    try {
        const response = await makeRequest('add_coupon', 'POST', {
            code: code,
            discount_percent: discount.toFixed(2)
        });

        if (response.status === 'success') {
            alert('Promo coupon registered successfully!');
            closeModal('coupon-modal');
            loadAdminCoupons();
        } else {
            alert(response.message || 'Failed to add coupon.');
        }
    } catch (e) {
        alert('Server validation error.');
    }
}

async function toggleCouponActive(code, checked) {
    try {
        const response = await makeRequest('toggle_coupon', 'POST', {
            code: code,
            is_active: checked ? '1' : '0'
        });
        if (response.status === 'success') {
            loadAdminCoupons();
        }
    } catch (e) {
        alert('Could not toggle coupon state.');
    }
}

// Announcements Tab
async function loadAdminAnnouncementsList() {
    const container = document.getElementById('admin-announcements-list');
    if (!container) return;

    try {
        const list = await makeRequest('get_announcements');
        if (list.length === 0) {
            container.innerHTML = '<p style="color:var(--text-secondary); text-align:center; padding: 20px 0;">No broadcast logs.</p>';
            return;
        }

        container.innerHTML = list.map(a => `
            <div style="padding:16px; border-radius:var(--radius-sm); border:1px solid var(--border-color); background: var(--bg-primary); position:relative;">
                <div style="font-size:14px; font-weight:500; color:var(--text-primary); margin-bottom:8px;">${a.message}</div>
                <div style="font-size:11px; color:var(--text-secondary);"><i class="fa-regular fa-clock"></i> Broadcasted: ${a.timestamp}</div>
            </div>
        `).join('');
    } catch (e) {
        container.innerHTML = '<p>Failed to load broadcast history.</p>';
    }
}

async function handleAnnouncementSubmit(event) {
    event.preventDefault();
    const msgInput = document.getElementById('announce-message');
    const message = msgInput.value.trim();

    try {
        const response = await makeRequest('add_announcement', 'POST', { message });
        if (response.status === 'success') {
            alert('Custom announcement ticker broadcasted live!');
            msgInput.value = '';
            loadAdminAnnouncementsList();
            fetchAnnouncements();
        } else {
            alert(response.message || 'Failed to broadcast.');
        }
    } catch (e) {
        alert('Broadcast failed.');
    }
}

// Banner Settings Tab
async function handleBannerUpdate(event) {
    event.preventDefault();
    const text = document.getElementById('banner-text-input').value.trim();
    const img = document.getElementById('banner-img-input').value.trim();

    try {
        const response = await makeRequest('update_banner', 'POST', {
            banner_text: text,
            banner_image_url: img
        });

        if (response.status === 'success') {
            alert('Hero banner configuration updated successfully!');
            document.getElementById('banner-preview-text').innerText = text;
        } else {
            alert(response.message || 'Failed to save configuration.');
        }
    } catch (e) {
        alert('Server communication error.');
    }
}

// Customers Tab
async function loadAdminCustomers() {
    const tbody = document.getElementById('admin-customers-table-body');
    if (!tbody) return;

    try {
        const list = await makeRequest('get_customers');

        if (list.length === 0) {
            tbody.innerHTML = '<tr><td colspan="6" class="text-center" style="color:var(--text-secondary); padding:20px;">No customers registered in database yet.</td></tr>';
            return;
        }

        tbody.innerHTML = list.map(c => {
            const walletBalance = c.wallet ? parseFloat(c.wallet.balance).toFixed(2) : '0.00';
            return `
                <tr>
                    <td style="font-weight:700;">#${c.id}</td>
                    <td style="font-weight:600;">${c.username}</td>
                    <td>${c.email}</td>
                    <td style="font-size:13px;">${c.phone || 'N/A'}</td>
                    <td style="max-width:250px; font-size:12px; color:var(--text-secondary); overflow:hidden; text-overflow:ellipsis; white-space:nowrap;" title="${c.address}">${c.address || 'N/A'}</td>
                    <td style="font-weight:600; color:var(--stc-purple);">${formatCurrency(walletBalance)}</td>
                </tr>
            `;
        }).join('');
    } catch (e) {
        tbody.innerHTML = '<tr><td colspan="6" class="text-center">Failed to retrieve customer accounts.</td></tr>';
    }
}
