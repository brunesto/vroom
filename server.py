import http.server
import subprocess
import sys
from urllib.parse import urlparse, parse_qs

# CONFIGURATION
PORT = 3000
BINARY_PATH = "bin/vroom"  # Make sure this is executable (chmod +x)
BINARY_ARGS = [""]  # Add arguments here, e.g. ["-g", "4"]
BLACKLISTED_PARAMS = ["i", "input", "o", "output"]

class JSONWrapperHandler(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        # Parse query parameters
        query = urlparse(self.path).query
        query_params = parse_qs(query)

        # Check for blacklisted parameters
        for param in query_params:
            if param in BLACKLISTED_PARAMS:
                self.send_error(400, f"Parameter '{param}' is not allowed")
                return

        request_args = []
        for k, v in query_params.items():
            prefix = "-" if len(k) == 1 else "--"
            flag = k if k.startswith("-") else f"{prefix}{k}"
            for val in v:
                request_args.append(flag)
                if val:
                    request_args.append(val)

        # 1. READ THE REQUEST BODY
        content_length = int(self.headers.get('Content-Length', 0))
        if content_length == 0:
            self.send_error(400, "No content sent")
            return
            
        request_body = self.rfile.read(content_length)

        try:
            # 2. RUN THE BINARY
            # We pass the body to the binary's STDIN and capture STDOUT
            process = subprocess.Popen(
                [BINARY_PATH] + [x for x in BINARY_ARGS if x] + request_args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=sys.stderr,
                shell=False
            )
            
            # communicate() handles writing to stdin and reading from stdout safely
            stdout_data, stderr_data = process.communicate(input=request_body)

            # 3. SEND THE RESPONSE
            if process.returncode != 0:
                # If binary crashed, send 500 and the stderr log
                self.send_response(500)
                self.end_headers()
                if stderr_data:
                    self.wfile.write(stderr_data)
            else:
                # Success! Send 200 and the JSON output
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(stdout_data)

        except Exception as e:
            self.send_error(500, f"Server Error: {str(e)}")

if __name__ == "__main__":
    server_address = ('0.0.0.0', PORT)
    httpd = http.server.HTTPServer(server_address, JSONWrapperHandler)
    print(f"Serving HTTP on port {PORT} wrapping {BINARY_PATH}...")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server.")
