<?php
// Define the target directory for file uploads
$targetDir = "/Users/abablil/Desktop/webserv/upload/";

// Ensure the upload directory exists
if (!file_exists($targetDir)) {
	mkdir($targetDir, 0777, true);
}

// Handle file upload
$message = "";
if ($_SERVER["REQUEST_METHOD"] == "POST") {
	if (isset($_FILES["file"]) && $_FILES["file"]["error"] == 0) {
		$fileName = basename($_FILES["file"]["name"]);
		$targetFilePath = $targetDir . $fileName;

		// Check file type (optional)
		// Move the uploaded file to the target directory
		if (move_uploaded_file($_FILES["file"]["tmp_name"], $targetFilePath)) {
			$message = "File uploaded successfully: $fileName";
		} else {
			$message = "Error uploading the file.";
		}

	} else {
		$message = "No file uploaded or an error occurred.";
	}
}
?>


<!DOCTYPE html>
<html lang="en">

<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>File Upload</title>
	<style>
		body {
			font-family: Arial, sans-serif;
			margin: 50px;
			text-align: center;
		}

		form {
			border: 1px solid #ccc;
			padding: 20px;
			border-radius: 10px;
			display: inline-block;
			box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
		}

		input[type="file"] {
			margin-bottom: 10px;
		}

		button {
			background-color: #007BFF;
			color: #fff;
			border: none;
			padding: 10px 20px;
			border-radius: 5px;
			cursor: pointer;
		}

		button:hover {
			background-color: #0056b3;
		}

		.message {
			margin-top: 20px;
			color: green;
		}
	</style>
</head>

<body>
	<h1>Upload a File</h1>
	<form action="upload" method="POST" enctype="multipart/form-data">
		<input type="file" name="file" required>
		<br>
		<button type="submit">Upload</button>
	</form>

	<?php if (!empty($message)): ?>
		<div class="message">
			<?php echo htmlspecialchars($message); ?>
		</div>
	<?php endif; ?>
</body>

</html>