<?php
session_start();

if (!isset($_SESSION["loggedin"])) {
	header("Location: /login");
	exit;
}

if (isset($_POST["logout"])) {
	session_unset();
	session_destroy();
	header("Location: /login");
	exit;
}

// Check if the theme cookie is set
$theme = isset($_COOKIE['theme']) ? $_COOKIE['theme'] : 'light';

// Handle theme toggle
if (isset($_POST['theme'])) {
	$newTheme = $_POST['theme'] === 'dark' ? 'dark' : 'light';
	setcookie('theme', $newTheme, time() + (86400 * 30), "/");
	header("Refresh:0");
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
	<link rel="icon" type="image/png" href="./icon.png" />
</head>

<body class="<?php echo $theme === 'dark' ? 'bg-gray-900 text-white' : 'bg-gray-50 text-black'; ?> min-h-screen">
	<!-- Navigation Bar -->
	<nav class="<?php echo $theme === 'dark' ? 'bg-gray-800' : 'bg-white'; ?> shadow-sm">
		<div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
			<div class="flex justify-between h-16">
				<div class="flex items-center">
					<span
						class="text-xl font-semibold <?php echo $theme === 'dark' ? 'text-white' : 'text-gray-800'; ?>">Dashboard</span>
				</div>
				<div class="flex items-center">
					<form action="" method="post" class="ml-4">
						<input type="hidden" name="logout" value="true">
						<button type="submit"
							class="inline-flex items-center px-4 py-2 border border-transparent text-sm font-medium rounded-md text-white bg-red-600 hover:bg-red-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-red-500 transition-colors duration-150">
							<svg class="h-4 w-4 mr-1.5" fill="none" stroke="currentColor" viewBox="0 0 24 24">
								<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2"
									d="M17 16l4-4m0 0l-4-4m4 4H7m6 4v1a3 3 0 01-3 3H6a3 3 0 01-3-3V7a3 3 0 013-3h4a3 3 0 013 3v1">
								</path>
							</svg>
							Logout
						</button>
					</form>
					<form action="" method="post" class="ml-4">
						<input type="hidden" name="theme" value="<?php echo $theme === 'dark' ? 'light' : 'dark'; ?>">
						<button type="submit"
							class="inline-flex items-center px-4 py-2 border border-transparent text-sm font-medium rounded-md text-white <?php echo $theme === 'dark' ? 'bg-gray-700 hover:bg-gray-600' : 'bg-blue-600 hover:bg-blue-700'; ?> focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 transition-colors duration-150">
							<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"
								fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"
								stroke-linejoin="round"
								class="h-4 w-4 mr-1.5 <?php echo $theme === 'dark' ? 'text-gray-300' : 'text-white'; ?>">
								<path stroke="none" d="M0 0h24v24H0z" fill="none" />
								<path d="M12 12m-3 0a3 3 0 1 0 6 0a3 3 0 1 0 -6 0" />
								<path d="M12 5l0 -2" />
								<path d="M17 7l1.4 -1.4" />
								<path d="M19 12l2 0" />
								<path d="M17 17l1.4 1.4" />
								<path d="M12 19l0 2" />
								<path d="M7 17l-1.4 1.4" />
								<path d="M6 12l-2 0" />
								<path d="M7 7l-1.4 -1.4" />
							</svg>
							<?php echo $theme === 'dark' ? 'Light Mode' : 'Dark Mode'; ?>
						</button>
					</form>
				</div>
			</div>
		</div>
	</nav>

	<!-- Main Content -->
	<main class="max-w-7xl mx-auto py-12 px-4 sm:px-6 lg:px-8">
		<div
			class="<?php echo $theme === 'dark' ? 'bg-gray-800' : 'bg-white'; ?> rounded-2xl shadow-xl overflow-hidden">
			<div class="p-8 sm:p-12">
				<div
					class="<?php echo $theme === 'dark' ? 'bg-gray-700' : 'bg-gradient-to-r from-blue-500 to-blue-600'; ?> -m-8 sm:-m-12 p-8 sm:p-12 mb-8 sm:mb-12">
					<h1
						class="text-3xl sm:text-4xl font-bold <?php echo $theme === 'dark' ? 'text-white' : 'text-white'; ?> mb-4">
						Welcome back,
						<?php echo isset($_SESSION['username']) ? htmlspecialchars($_SESSION['username']) : 'Guest'; ?>!
					</h1>
					<p class="<?php echo $theme === 'dark' ? 'text-gray-300' : 'text-blue-100'; ?> text-lg">
						<?php echo date('l, F jS, Y'); ?>
					</p>
				</div>

				<div class="space-y-6">
					<div
						class="<?php echo $theme === 'dark' ? 'bg-gray-700' : 'bg-blue-50'; ?> rounded-xl p-6 border <?php echo $theme === 'dark' ? 'border-gray-600' : 'border-blue-100'; ?>">
						<div class="flex items-center">
							<div class="flex-shrink-0">
								<svg class="h-8 w-8 <?php echo $theme === 'dark' ? 'text-gray-300' : 'text-blue-600'; ?>"
									fill="none" stroke="currentColor" viewBox="0 0 24 24">
									<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2"
										d="M13 10V3L4 14h7v7l9-11h-7z">
									</path>
								</svg>
							</div>
							<div class="ml-4">
								<h2
									class="text-xl font-semibold <?php echo $theme === 'dark' ? 'text-gray-100' : 'text-gray-900'; ?>">
									Quick Start</h2>
								<p class="mt-2 <?php echo $theme === 'dark' ? 'text-gray-400' : 'text-gray-600'; ?>">
									We're glad to see you again. Your dashboard is ready for you to explore.
								</p>
							</div>
						</div>
					</div>

					<div class="grid grid-cols-1 md:grid-cols-3 gap-6">
						<div
							class="<?php echo $theme === 'dark' ? 'bg-gray-700' : 'bg-white'; ?> rounded-xl p-6 border <?php echo $theme === 'dark' ? 'border-gray-600' : 'border-gray-200'; ?> shadow-sm hover:shadow-md transition-shadow duration-150">
							<div class="<?php echo $theme === 'dark' ? 'text-gray-400' : 'text-gray-500'; ?> mb-2">Total
								Views</div>
							<div
								class="text-2xl font-bold <?php echo $theme === 'dark' ? 'text-gray-100' : 'text-gray-900'; ?>">
								1,234</div>
						</div>
						<div
							class="<?php echo $theme === 'dark' ? 'bg-gray-700' : 'bg-white'; ?> rounded-xl p-6 border <?php echo $theme === 'dark' ? 'border-gray-600' : 'border-gray-200'; ?> shadow-sm hover:shadow-md transition-shadow duration-150">
							<div class="<?php echo $theme === 'dark' ? 'text-gray-400' : 'text-gray-500'; ?> mb-2">
								Active Projects</div>
							<div
								class="text-2xl font-bold <?php echo $theme === 'dark' ? 'text-gray-100' : 'text-gray-900'; ?>">
								12</div>
						</div>
						<div
							class="<?php echo $theme === 'dark' ? 'bg-gray-700' : 'bg-white'; ?> rounded-xl p-6 border <?php echo $theme === 'dark' ? 'border-gray-600' : 'border-gray-200'; ?> shadow-sm hover:shadow-md transition-shadow duration-150">
							<div class="<?php echo $theme === 'dark' ? 'text-gray-400' : 'text-gray-500'; ?> mb-2">Team
								Members</div>
							<div
								class="text-2xl font-bold <?php echo $theme === 'dark' ? 'text-gray-100' : 'text-gray-900'; ?>">
								8</div>
						</div>
					</div>
				</div>
			</div>
		</div>
	</main>
</body>

</html>