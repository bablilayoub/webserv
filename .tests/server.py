from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler

class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        # Log request line and headers
        print("HTTP Request Received:")
        print(self.requestline)
        # for header, value in self.headers.items():
        #     print("{}: {}".format(header, value))
        
        # Read and log the body
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length)
        print("\nBody:\n" + body)
        print("end of Data .")

        # Send a basic response
        self.send_response(200)
        self.end_headers()
        self.wfile.write("Received")

# Start the server
port = 8080
print("Server is listening on port {}...".format(port))
httpd = HTTPServer(('0.0.0.0', port), SimpleHTTPRequestHandler)
httpd.serve_forever()
