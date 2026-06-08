# Live Deployment Guide for STC Project

This project is a full application with both frontend and backend. To make it work live for everyone on Netlify, a static simulation mode is already built into `js/app.js`.

## What to deploy on Netlify
Use the prepared `netlify_build` folder. It contains only the frontend and a browser-side localStorage simulation engine, so it works as a static site without a backend.

- `netlify_build/index.html`
- `netlify_build/customer.html`
- `netlify_build/admin.html`
- `netlify_build/css/`
- `netlify_build/js/`
- `netlify_build/images/`

## When to deploy the full app instead
If you want the original backend-powered version with real file-based persistence and C++/Python support, deploy the full project folder to a Python-capable host.

## Recommended hosting options
Use a host that can run a Python web server, such as:
- Render (https://render.com)
- Railway (https://railway.app)
- PythonAnywhere (https://www.pythonanywhere.com)
- A VPS / cloud server (DigitalOcean, AWS, etc.)

## Deployment setup
1. Create an account on the host.
2. Connect your GitHub repository or upload the project folder.
3. Ensure the root of the app contains:
   - `server.py`
   - `Procfile`
   - `requirements.txt`
   - `runtime.txt`
4. Set the start command to:
   ```
   python server.py
   ```
5. Deploy.

## What will work
- The app will serve the frontend pages.
- The backend routes under `/cgi-bin/` will run via the Python fallback server.
- Customers can log in, register, place orders, load wallet balance, and use the admin pages.

## Notes
- Your current local file `run_full_app.bat` is only for running locally on Windows.
- For live hosting, do not use Netlify alone.
- If your host allows runtime files, `runtime.txt` requests Python 3.11.

## If you want the easiest live setup
Use Render or Railway and choose a Python web service. Then deploy the full project root and use `python server.py` as the startup command.
