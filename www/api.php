<?php

header("Content-Type: application/json");

$targetDir = $_ENV['ROOT_FOLDER'];

if ($_SERVER["REQUEST_METHOD"] !== "DELETE") {
	http_response_code(405);
	echo json_encode(array("message" => "Only DELETE method is allowed"));
	exit;
}

if (!file_exists($targetDir)) {
	http_response_code(404);
	echo json_encode(array("message" => "Directory does not exist"));
	exit;
}

$path = isset($_GET["path"]) ? $_GET["path"] : "";

if (empty($path)) {
	http_response_code(400);
	echo json_encode(array("message" => "Please enter a valid path"));
	exit;
}

$full_path = realpath($targetDir . "/" . $path);

if (!file_exists(filename: $full_path)) {
	http_response_code(404);
	echo json_encode(array("message" => "File or directory does not exist"));
	exit;
}

if (strpos($full_path, $targetDir) !== 0) {
	http_response_code(403);
	echo json_encode(array("message" => "Forbidden"));
	exit;
}

if (is_dir($full_path)) {
	if (rmdir($full_path)) {
		http_response_code(200);
		echo json_encode(array("message" => "Directory deleted successfully"));
	} else {
		http_response_code(500);
		echo json_encode(array("message" => "Failed to delete directory"));
	}
} else {
	if (unlink($full_path)) {
		http_response_code(200);
		echo json_encode(array("message" => "File deleted successfully"));
	} else {
		http_response_code(500);
		echo json_encode(array("message" => "Failed to delete file"));
	}
}

?>