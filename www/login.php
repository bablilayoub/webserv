<?php
session_start();

if (isset($_SESSION["loggedin"]) && $_SESSION["loggedin"] === true) {
    header("Location: /dashboard");
    exit;
}

if (isset($_POST["username"]) && isset($_POST["password"])) {
    $_SESSION["loggedin"] = true;
    $_SESSION["username"] = $_POST["username"];
    $_SESSION["password"] = $_POST["password"];
    header("Location: /dashboard");
    exit;
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login</title>
    <link rel="icon" type="image/png" href="./icon.png" />
    <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="bg-gray-50 min-h-screen">
    <div class="min-h-screen flex flex-col">
        <!-- Nav placeholder for consistency -->
        <nav class="bg-white shadow-sm">
            <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
                <div class="flex justify-between h-16">
                    <div class="flex items-center">
                        <span class="text-xl font-semibold text-gray-800">Welcome</span>
                    </div>
                </div>
            </div>
        </nav>

        <!-- Main Content -->
        <div class="flex-grow flex items-center justify-center py-12 px-4 sm:px-6 lg:px-8">
            <div class="max-w-md w-full">
                <div class="bg-white rounded-2xl shadow-xl overflow-hidden">
                    <!-- Gradient Header -->
                    <div class="bg-gradient-to-r from-blue-500 to-blue-600 p-8">
                        <h2 class="text-3xl font-bold text-white">
                            Welcome back
                        </h2>
                        <p class="mt-2 text-blue-100">
                            Please sign in to your account
                        </p>
                    </div>

                    <!-- Login Form -->
                    <div class="p-8">
                        <form method="POST" action="" class="space-y-6">
                            <div class="space-y-4">
                                <div>
                                    <label for="username" class="block text-sm font-medium text-gray-700">
                                        Username
                                    </label>
                                    <div class="mt-1 relative">
                                        <input id="username" 
                                               name="username" 
                                               type="text" 
                                               required 
                                               class="appearance-none block w-full px-3 py-2 border border-gray-300 rounded-lg shadow-sm placeholder-gray-400 focus:outline-none focus:ring-blue-500 focus:border-blue-500 sm:text-sm"
                                               placeholder="Enter your username">
                                    </div>
                                </div>

                                <div>
                                    <label for="password" class="block text-sm font-medium text-gray-700">
                                        Password
                                    </label>
                                    <div class="mt-1 relative">
                                        <input id="password" 
                                               name="password" 
                                               type="password" 
                                               required 
                                               class="appearance-none block w-full px-3 py-2 border border-gray-300 rounded-lg shadow-sm placeholder-gray-400 focus:outline-none focus:ring-blue-500 focus:border-blue-500 sm:text-sm"
                                               placeholder="Enter your password">
                                    </div>
                                </div>
                            </div>

                            <div>
                                <button type="submit" 
                                        class="w-full flex justify-center py-2.5 px-4 border border-transparent rounded-lg shadow-sm text-sm font-medium text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 transition-all duration-150 hover:shadow-lg">
                                    <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                        <path stroke-linecap="round" 
                                              stroke-linejoin="round" 
                                              stroke-width="2" 
                                              d="M11 16l-4-4m0 0l4-4m-4 4h14m-5 4v1a3 3 0 01-3 3H6a3 3 0 01-3-3V7a3 3 0 013-3h7a3 3 0 013 3v1">
                                        </path>
                                    </svg>
                                    Sign in
                                </button>
                            </div>
                        </form>
                    </div>
                </div>

                <!-- Additional Info Box -->
                <div class="mt-6">
                    <div class="bg-blue-50 rounded-xl p-6 border border-blue-100">
                        <div class="flex items-center">
                            <div class="flex-shrink-0">
                                <svg class="h-6 w-6 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" 
                                          stroke-linejoin="round" 
                                          stroke-width="2" 
                                          d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z">
                                    </path>
                                </svg>
                            </div>
                            <div class="ml-3">
                                <p class="text-sm text-gray-600">
                                    Need help? Contact support for assistance with your account.
                                </p>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</body>
</html>