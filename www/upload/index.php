<?php


if (isset($_SERVER['PATH_TRANSLATED'])) {
    echo "PATH_TRANSLATED: " . htmlspecialchars($_SERVER['PATH_TRANSLATED']) . "<br>";
} else {
    echo "No PATH_TRANSLATED available.";
}

if (isset($_SERVER['PATH_INFO'])) {
    echo "PATH_INFO: " . htmlspecialchars($_SERVER['PATH_INFO']);
} else {
    echo "No PATH_INFO available.";
}
?>
