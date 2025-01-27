#!/usr/bin/env python3
import cgi
import cgitb

# Enable CGI error handling
cgitb.enable()

# Initialize variables
name = ""
email = ""
message = ""

# Get form data
form = cgi.FieldStorage()
if form.getvalue("name") and form.getvalue("email"):
    name = form.getvalue("name")
    email = form.getvalue("email")
    message = f"Thank you, {name}! Your email address ({email}) has been received."

# HTML content for the contact form
print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Contact Form Python</title>
    <link rel="icon" type="image/png" href="./icon.png" />
    <style>
        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f7fc;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }}
        .container {{
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 400px;
        }}
        h1 {{
            font-size: 24px;
            text-align: center;
            color: #333;
        }}
        label {{
            font-size: 16px;
            margin-bottom: 8px;
            color: #555;
        }}
        input[type="text"], input[type="email"] {{
            width: 100%;
            padding: 12px;
            margin: 8px 0;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 14px;
        }}
        input[type="submit"] {{
            width: 100%;
            padding: 12px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
        }}
        input[type="submit"]:hover {{
            background-color: #0056b3;
        }}
        p {{
            font-size: 16px;
            color: #28a745;
            text-align: center;
            margin-top: 15px;
        }}
    </style>
</head>
<body>

    <div class="container">
        <h1>Contact Form Python</h1>
        <form method="post" action="">
            <label for="name">Name:</label><br>
            <input type="text" id="name" name="name" value="{name}" required><br><br>
            <label for="email">Email:</label><br>
            <input type="email" id="email" name="email" value="{email}" required><br><br>
            <input type="submit" value="Submit">
        </form>

        {f"<p>{message}</p>" if message else ""}
    </div>

</body>
</html>
""")
