<?php

session_start();

if (!isset($_SESSION["loggedin"])) {
	header(header: "Location: /login");
	exit;
}

if (isset($_POST["logout"])) {
	session_unset();
	session_destroy();
	header("Location: /login");
	exit;
}
?>

<!DOCTYPE html>
<html lang="en">

<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Welcome</title>
	<script src="https://cdn.tailwindcss.com"></script>

	<style>
		@keyframes fadeInDown {
			0% {
				opacity: 0;
				transform: translateY(-20px);
			}

			100% {
				opacity: 1;
				transform: translateY(0);
			}
		}

		@keyframes fadeInUp {
			0% {
				opacity: 0;
				transform: translateY(20px);
			}

			100% {
				opacity: 1;
				transform: translateY(0);
			}
		}

		.animate-fadeInDown {
			animation: fadeInDown 0.8s ease-out;
		}

		.animate-fadeInUp {
			animation: fadeInUp 1s ease-out;
		}
	</style>
</head>

<body
	class="bg-gradient-to-br from-purple-600 via-blue-500 to-indigo-900 text-white font-sans min-h-screen flex items-center justify-center">
	<div class="bg-white bg-opacity-10 backdrop-blur-md rounded-2xl p-8 shadow-lg text-center">
		<h1 class="text-4xl font-extrabold mb-4 text-gray-100 animate-fadeInDown">
			Welcome back, <?php echo isset($_SESSION['username']) ? $_SESSION['username'] : 'Guest'; ?>!
		</h1>
		<p class="text-lg text-gray-300 mb-6 animate-fadeInUp">
			We're glad to see you again. Click below to log out and return to the login page.
		</p>
		<form action="" method="post">
			<input type="hidden" name="logout" value="true">
			<button type="submit"
				class="px-6 py-3 bg-pink-500 rounded-full text-white font-semibold text-lg shadow-lg hover:bg-pink-400 focus:outline-none focus:ring-4 focus:ring-pink-300 transition-transform transform hover:scale-105 animate-fadeInUp">
				Logout
			</button>
		</form>
	</div>
</body>

</html>