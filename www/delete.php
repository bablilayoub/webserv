<?php
$targetDir = "/Users/abablil/Desktop/webserv/upload/";

// Ensure the upload directory exists
if (!file_exists($targetDir)) {
	mkdir($targetDir, 0777, true);
}

$messageType = "";
$message = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") { // Change to POST since DELETE is not commonly used in forms
	// Sanitize the filename input
	$filename = basename($_POST["filename"]);
	$filePath = $targetDir . $filename;

	if (empty($filename)) {
		$messageType = "error";
		$message = "Please enter a file name.";
	} else if (!file_exists($filePath)) {
		$messageType = "error";
		$message = "File not found: " . htmlspecialchars($filename);
	} else {
		// Attempt to delete the file
		if (unlink($filePath)) {
			$messageType = "success";
			$message = "File deleted successfully: " . htmlspecialchars($filename);
		} else {
			$messageType = "error";
			$message = "Failed to delete the file: " . htmlspecialchars($filename);
		}
	}
}
?>

<!DOCTYPE html>
<html lang="en">

<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Delete file by name</title>
	<link rel="icon" type="image/png" href="./icon.png" />
	<script src="https://cdn.tailwindcss.com"></script>
</head>

<body class="bg-gray-100 min-h-screen flex items-center justify-center">
	<div class="bg-white p-8 rounded-lg shadow-lg min-w-[300px] max-w-[500px] w-full">
		<h1 class="text-2xl font-bold text-gray-800 text-center mb-4">Delete file by name</h1>
		<p class="text-sm text-gray-600 text-center mb-6">Enter the name of the file you want to delete</p>
		<form action="" method="POST" class="space-y-4">
			<input type="text" name="filename" placeholder="File name"
				class="w-full border border-gray-300 rounded-lg p-2 focus:outline-none focus:border-red-500 transition">
			<button type="submit"
				class="w-full bg-red-500 text-white py-2 rounded-lg hover:bg-red-600 transition">Delete</button>
		</form>

		<?php if (!empty($message)): ?>
			<div
				class="mt-4 p-4 text-sm rounded-lg <?php echo $messageType === 'success' ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800'; ?>">
				<?php echo htmlspecialchars($message); ?>
			</div>
		<?php endif; ?>
	</div>

</body>

</html>