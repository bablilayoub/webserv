# Webserv - 42 / 1337 Project

Webserv is a lightweight web server designed to handle common HTTP methods efficiently. It allows users to browse files directly from a web browser while supporting essential web server functionalities.

## Features

- ✅ Non-blocking sockets (handles multiple clients simultaneously)
- ✅ Supports HTTP methods: `GET`, `POST`, `DELETE`
- ✅ Autoindex (directory listing)
- ✅ File upload
- ✅ CGI support (Common Gateway Interface)
- ✅ Redirections
- ✅ Sessions & Cookies
- ✅ Basic authentication
- ✅ Chunked transfer encoding
- ✅ Custom error pages
- ✅ Configurable server settings
- ✅ Streaming responses (chunked response handling)
- ✅ Multiple server blocks
- ✅ Client body size limitation
- ✅ Timeout control for CGI scripts
- ✅ Logging and error reporting
- ✅ Support for static and dynamic content

## Requirements

- C++98
- CMake
- Make
- A C++ compiler

## Installation

```bash
git clone https://github.com/bablilayoub/webserv
cd webserv
make
```

## Usage

Start the server with a custom configuration file:
```bash
./webserv /path/to/config_file.conf
```
*If no configuration file is provided, the server will use default settings.*

To test the server locally, open a browser and navigate to:
```
http://127.0.0.1:8080
```

To upload a file via `curl`:
```bash
curl -X POST -F "file=@/path/to/your/file" http://127.0.0.1/upload
```

## Configuration

Webserv uses a simple text-based configuration file structured as follows:

```conf
server {
    host 127.0.0.1;
    listen 8080;
    limit_client_body_size 1000000;
    root_folder /path/to/your/root/folder;
    error_page 404 /custom_404.html;

    location / {
        index index.html;
        autoindex on;
        upload_dir /path/to/your/upload/folder;
        accepted_methods GET POST;
        cgi_extensions php py;
        php_cgi_path /path/to/your/php-interpreter;
        python_cgi_path /path/to/your/python-interpreter;
        cgi_timeout 5;
    }

    location /login {
        index login.php;
        upload_dir /path/to/your/upload/folder;
        accepted_methods GET POST;
        cgi_extensions php;
        php_cgi_path /path/to/your/php-interpreter;
        cgi_timeout 10;
    }

    location /redirect {
        redirect 301 /redirected;
    }

    location /static {
        root_folder /var/www/static;
        accepted_methods GET;
    }
}
```

### Example Scenarios

- **Hosting a Static Website:**
  - Place HTML, CSS, and JS files in `/var/www/static` and configure a `/static` route.
  - Access the site via `http://127.0.0.1:8080/static/index.html`

- **Enabling File Uploads:**
  - Ensure `upload_dir` is properly set.
  - Use `curl` or a web form to send files to the server.

- **Running a PHP-based Login System:**
  - Configure a `/login` location with `php_cgi_path`.
  - Ensure the PHP interpreter is correctly installed.

## Authors

- [Ayoub Bablil](https://github.com/bablilayoub)
- [Achraf Laalalma](https://github.com/Achraflaalalma)
- [Aimen Taoussi](https://github.com/REDX-at)

## License

This project is open-source and available under the MIT License.
