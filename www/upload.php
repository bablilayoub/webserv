<?php

$targetDir = "/Users/abablil/Desktop/webserv/upload/";

if (!file_exists($targetDir)) {
	mkdir($targetDir, 0777, true);
}

$messageType = "";
$message = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
	if (isset($_FILES["file"]) && $_FILES["file"]["error"] == 0) {
		$fileName = basename(path: $_FILES["file"]["name"]);
		$targetFilePath = $targetDir . $fileName;

		if (move_uploaded_file($_FILES["file"]["tmp_name"], $targetFilePath)) {
			$messageType = "success";
			$message = "File uploaded successfully: $fileName";
		} else {
			$messageType = "error";
			$message = "Error uploading the file.";
		}
	} else {
		$messageType = "error";
		$message = "No file uploaded or an error occurred: " . $_FILES["file"]["error"];
	}
}
?>

<!DOCTYPE html>
<html lang="en">

<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>PHP File Upload</title>
	<link rel="icon" type="image/png" href="./icon.png" />
	<script src="https://cdn.tailwindcss.com"></script>
</head>

<body class="bg-gray-100 min-h-screen flex items-center justify-center">
	<div class="bg-white p-8 rounded-lg shadow-lg min-w-[300px] max-w-[500px] w-full">
		<h1 class="text-2xl font-bold text-gray-800 text-center mb-4">PHP File Upload</h1>
		<p class="text-sm text-gray-600 text-center mb-6">Drag and drop your file below or click to select one.</p>
		<form action="" method="POST" enctype="multipart/form-data" id="uploadForm" class="space-y-4">
			<div id="dropZone"
				class="border-2 border-dashed border-blue-500 rounded-lg py-10 flex flex-col items-center space-y-2 transition hover:bg-blue-50 focus:outline-none focus:ring-2 focus:ring-blue-500">
				<input type="file" id="file" name="file" class="hidden" required>
				<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none"
					stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
					class="h-12 w-12 text-blue-500">
					<path stroke="none" d="M0 0h24v24H0z" fill="none" />
					<path d="M4 17v2a2 2 0 0 0 2 2h12a2 2 0 0 0 2 -2v-2" />
					<path d="M7 9l5 -5l5 5" />
					<path d="M12 4l0 12" />
				</svg>
				<span id="fileName" class="text-sm text-gray-600">No file selected</span>
			</div>

			<button type="submit"
				class="w-full bg-blue-500 text-white py-2 rounded-lg hover:bg-blue-600 transition">Upload</button>
		</form>

		<?php if (!empty($message)): ?>
			<div
				class="mt-4 p-4 text-sm rounded-lg <?php echo $messageType === 'success' ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800'; ?>">
				<?php echo htmlspecialchars($message); ?>
			</div>
		<?php endif; ?>
	</div>

	<script>
		const dropZone = document.getElementById('dropZone');
		const fileInput = document.getElementById('file');
		const fileNameDisplay = document.getElementById('fileName');

		fileInput.addEventListener('change', () => {
			const file = fileInput.files[0];
			fileNameDisplay.textContent = file ? file.name.trim().length > 40 ? file.name.trim().substring(0, 40) + '...' : file.name.trim() : 'No file selected';
		});

		dropZone.addEventListener('dragover', (e) => {
			e.preventDefault();
			dropZone.classList.add('bg-blue-100');
		});

		dropZone.addEventListener('dragleave', () => {
			dropZone.classList.remove('bg-blue-100');
		});

		dropZone.addEventListener('drop', (e) => {
			e.preventDefault();
			dropZone.classList.remove('bg-blue-100');
			const file = e.dataTransfer.files[0];
			if (file) {
				fileInput.files = e.dataTransfer.files; // Set the file input value
				fileNameDisplay.textContent = file.name.trim().length > 40 ? file.name.trim().substring(0, 40) + '...' : file.name.trim();
			}
		});

		dropZone.addEventListener('click', () => fileInput.click());
	</script>
</body>

</html>