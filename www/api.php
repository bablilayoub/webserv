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

$filename = isset($_GET["filename"]) ? $_GET["filename"] : "";

$filename = basename($filename);

if (empty($filename)) {
	http_response_code(400);
	echo json_encode(array("message" => "Please enter a file name"));
	exit;
}

$filePath = realpath($targetDir . "/" . $filename);

if (!file_exists($filePath)) {
	http_response_code(404);
	echo json_encode(array("message" => "File not found"));
	exit;
}

if (unlink($filePath)) {
	http_response_code(200);
	echo json_encode(array("message" => "File deleted successfully"));
} else {
	http_response_code(500);
	echo json_encode(array("message" => "Failed to delete the file"));
}

?>