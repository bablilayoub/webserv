<?php

// Initialize variables
$name = "";
$email = "";
$message = "";

// Check if the form is submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Get the submitted values
    $name = htmlspecialchars($_POST['name'] ? $_POST['name'] : "");
    $email = htmlspecialchars($_POST['email'] ? $_POST['email'] : "");
    $message = "Thank you, $name! Your email address ($email) has been received.";
}

if ($_SERVER["REQUEST_METHOD"] == "GET" && isset($_GET['message'])) {
    $message = htmlspecialchars($_GET['message']);
}

// while (true) {}

?>

<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Contact Form PHP</title>
    <!-- <link rel="icon" type="image/png" href="./icon.png" /> -->
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f7fc;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        .container {
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 400px;
        }

        h1 {
            font-size: 24px;
            text-align: center;
            color: #333;
        }

        label {
            font-size: 16px;
            margin-bottom: 8px;
            color: #555;
        }

        input[type="text"],
        input[type="email"] {
            width: 100%;
            padding: 12px;
            margin: 8px 0;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 14px;
        }

        input[type="submit"] {
            width: 100%;
            padding: 12px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
        }

        input[type="submit"]:hover {
            background-color: #0056b3;
        }

        p {
            font-size: 16px;
            color: #28a745;
            text-align: center;
            margin-top: 15px;
        }
    </style>
</head>

<body>

    <div class="container">
        <h1>Contact Form PHP</h1>
        <form method="POST" action="">
            <label for="name">Name:</label><br>
            <input type="text" id="name" name="name" required><br><br>
            <label for="email">Email:</label><br>
            <input type="email" id="email" name="email" required><br><br>
            <input type="submit" value="Submit">
        </form>

        <?php
        // Display the thank-you message if the form is submitted
        if (!empty($message)) {
            echo "<p>$message</p>";
        }
        ?>
    </div>

</body>

</html>