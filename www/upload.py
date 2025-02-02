#!/usr/bin/env python3

import os
import cgi
import cgitb

# Enable debugging
cgitb.enable()

# Set upload directory
UPLOAD_DIR = "/Users/abablil/goinfre/upload/"
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Initialize message
message_type = ""
message = ""

# Handle file upload
form = cgi.FieldStorage()
if "file" in form:
    file_item = form["file"]
    if file_item.file and file_item.filename:
        file_path = os.path.join(UPLOAD_DIR, os.path.basename(file_item.filename))
        try:
            with open(file_path, "wb") as f:
                f.write(file_item.file.read())
            message_type = "success"
            message = f"File uploaded successfully: {os.path.basename(file_item.filename)}"
        except Exception as e:
            message_type = "error"
            message = f"Error uploading the file: {e}"
    else:
        message_type = "error"
        message = "No file selected or an error occurred."

# HTML Response
print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python File Upload</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <link rel="icon" type="image/png" href="./icon.png" />
</head>
<body class="bg-gray-50 min-h-screen">
    <div class="min-h-screen flex flex-col">
        <!-- Nav placeholder for consistency -->
        <nav class="bg-white shadow-sm">
            <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
                <div class="flex justify-between h-16">
                    <div class="flex items-center">
                        <span class="text-xl font-semibold text-gray-800">Python File Upload</span>
                    </div>
                </div>
            </div>
        </nav>

        <!-- Main Content -->
        <div class="flex-grow flex items-center justify-center py-12 px-4 sm:px-6 lg:px-8">
            <div class="max-w-md w-full">
                <div class="bg-white rounded-2xl shadow-xl overflow-hidden">
                    <!-- Gradient Header -->
                    <div class="bg-gradient-to-r from-blue-500 to-blue-600 p-8">
                        <h2 class="text-3xl font-bold text-white">
                            Python File Upload
                        </h2>
                        <p class="mt-2 text-blue-100">
                            Select or drag a file to upload
                        </p>
                    </div>

                    <!-- Upload Form -->
                    <div class="p-8">
                        <form action="" method="POST" enctype="multipart/form-data" id="uploadForm" class="space-y-6">
                            <div id="dropZone" class="border-2 border-dashed border-gray-300 rounded-lg p-6 flex flex-col items-center space-y-4 cursor-pointer hover:border-blue-500 transition-colors">
                                <input type="file" id="file" name="file" class="hidden" required>
                                <svg xmlns="http://www.w3.org/2000/svg" class="h-12 w-12 text-gray-400" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M7 16a4 4 0 01-.88-7.903A5 5 0 1115.9 6L16 6a5 5 0 011 9.9M15 13l-3-3m0 0l-3 3m3-3v12" />
                                </svg>
                                <div class="text-center">
                                    <span id="fileName" class="text-sm text-gray-600">No file selected</span>
                                </div>
                            </div>

                            <button type="submit" class="w-full flex justify-center py-2.5 px-4 border border-transparent rounded-lg shadow-sm text-sm font-medium text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 transition-all duration-150 hover:shadow-lg">
                                <svg class="w-5 h-5 mr-2" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16v1a3 3 0 003 3h10a3 3 0 003-3v-1m-4-8l-4-4m0 0l-4 4m4-4v12" />
                                </svg>
                                Upload File
                            </button>
                        </form>
""")

# Add success or error message dynamically
if message:
    if message_type == "success":
        print(f"""
                        <div class="mt-4">
                            <div class="rounded-lg p-4 bg-green-50">
                                <div class="flex">
                                    <div class="flex-shrink-0">
                                        <svg class="h-5 w-5 text-green-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7"/>
                                        </svg>
                                    </div>
                                    <div class="ml-3">
                                        <p class="text-sm text-green-800">{message}</p>
                                    </div>
                                </div>
                            </div>
                        </div>
        """)
    elif message_type == "error":
        print(f"""
                        <div class="mt-4">
                            <div class="rounded-lg p-4 bg-red-50">
                                <div class="flex">
                                    <div class="flex-shrink-0">
                                        <svg class="h-5 w-5 text-red-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"/>
                                        </svg>
                                    </div>
                                    <div class="ml-3">
                                        <p class="text-sm text-red-800">{message}</p>
                                    </div>
                                </div>
                            </div>
                        </div>
        """)

print("""
                    </div>
                </div>

                <!-- Additional Info Box -->
                <div class="mt-6">
                    <div class="bg-blue-50 rounded-xl p-6 border border-blue-100">
                        <div class="flex items-center">
                            <div class="flex-shrink-0">
                                <svg class="h-6 w-6 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"/>
                                </svg>
                            </div>
                            <div class="ml-3">
                                <p class="text-sm text-gray-600">
                                    Supported file types: All file formats are supported.
                                </p>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
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
            dropZone.classList.add('border-blue-500');
        });

        dropZone.addEventListener('dragleave', () => {
            dropZone.classList.remove('border-blue-500');
        });

        dropZone.addEventListener('drop', (e) => {
            e.preventDefault();
            dropZone.classList.remove('border-blue-500');
            const file = e.dataTransfer.files[0];
            if (file) {
                fileInput.files = e.dataTransfer.files;
                fileNameDisplay.textContent = file.name.trim().length > 40 ? file.name.trim().substring(0, 40) + '...' : file.name.trim();
            }
        });

        dropZone.addEventListener('click', () => fileInput.click());
    </script>
</body>
</html>
""")
