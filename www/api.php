<?php

header("Content-Type: application/json");
ini_set('display_errors', 0);

$targetDir = $_ENV['ROOT_FOLDER'];

if (empty($targetDir) || !is_dir($targetDir)) {
    http_response_code(500);
    echo json_encode(array("message" => "Server misconfiguration: ROOT_FOLDER not set or invalid"));
    exit;
}

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

function deleteDir($dir) {
    if (!is_dir($dir)) {
        return unlink($dir);
    }
    foreach (scandir($dir) as $file) {
        if ($file !== '.' && $file !== '..') {
            deleteDir($dir . DIRECTORY_SEPARATOR . $file);
        }
    }
    return rmdir($dir);
}

if (deleteDir($full_path)) {
    http_response_code(200);
    echo json_encode(array("message" => "File or directory deleted successfully"));
} else {
    http_response_code(500);
    echo json_encode(array("message" => "Failed to delete file or directory"));
}

?>