<?php

header("Content-Type: application/json");

$targetDir = "/Users/abablil/goinfre/upload";

// if directory does not exist, return response
if (!file_exists($targetDir)) {
	http_response_code(404);
	echo json_encode(array("message" => "Directory does not exist"));
	exit;
}

// get the filename from the request
$filename = isset($_GET["filename"]) ? $_GET["filename"] : "";

// if filename is empty, return response
if (empty($filename)) {
	http_response_code(400);
	echo json_encode(array("message" => "Please enter a file name"));
	exit;
}

$filePath = $targetDir . $filename;
if (!file_exists($filePath)) {
	http_response_code(404);
	echo json_encode(array("message" => "File not found"));
	exit;
} else {
	// Attempt to delete the file
	if (unlink($filePath)) {
		http_response_code(200);
		echo json_encode(array("message" => "File deleted successfully"));
		exit;
	} else {
		http_response_code(500);
		echo json_encode(array("message" => "Failed to delete the file"));
		exit;
	}
}
?>