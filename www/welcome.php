<?php
session_start();

if (!isset($_SESSION["loggedin"])) {
	header("Location: login.php");
	exit;
}

if (isset($_POST["logout"])) {
	session_unset();
	session_destroy();
	header("Location: login.php");
	exit;
}
?>

<!DOCTYPE html>
<html lang="en">

<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Welcome</title>
	<link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&display=swap" rel="stylesheet">
	<style>
		body {
			margin: 0;
			padding: 0;
			font-family: 'Poppins', sans-serif;
			background: linear-gradient(135deg, #6a11cb, #2575fc);
			color: #fff;
			display: flex;
			justify-content: center;
			align-items: center;
			height: 100vh;
			overflow: hidden;
		}

		.welcome-container {
			text-align: center;
			padding: 2rem;
			background: rgba(255, 255, 255, 0.1);
			border-radius: 20px;
			box-shadow: 0 8px 30px rgba(0, 0, 0, 0.2);
			backdrop-filter: blur(10px);
		}

		.welcome-title {
			font-size: 3rem;
			font-weight: 600;
			margin-bottom: 1rem;
			animation: fadeInDown 1s ease;
		}

		.welcome-message {
			font-size: 1.2rem;
			font-weight: 400;
			margin-bottom: 2rem;
			animation: fadeInUp 1.2s ease;
		}

		.action-button {
			font-size: 1rem;
			font-weight: 600;
			color: #fff;
			background: #ff7eb3;
			padding: 0.8rem 2rem;
			border: none;
			border-radius: 25px;
			cursor: pointer;
			transition: all 0.3s ease;
			text-decoration: none;
			display: inline-block;
			animation: fadeInUp 1.5s ease;
		}

		.action-button:hover {
			background: #ff4d89;
			transform: scale(1.05);
		}

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
	</style>
</head>

<body>
	<div class="welcome-container">
		<h1 class="welcome-title">Welcome back
			<?php echo isset($_SESSION['username']) ? $_SESSION['username'] : 'Guest'; ?>!
		</h1>
		<p class="welcome-message">
		<form action="" method="post">
			<input type="hidden" name="logout" value="true">
			<button type="submit" class="action-button">Logout</button>
		</form>
	</div>
</body>

</html>