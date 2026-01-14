import http.server
import subprocess
import sys
import argparse
from urllib.parse import urlparse, parse_qs
import pathlib

# Parse command line arguments
parser = argparse.ArgumentParser(description="HTTP Server Wrapper")
parser.add_argument("--port", type=int, default=3000, help="Port to serve")
parser.add_argument("--exec", default="", help="Path to the binary")
parser.add_argument("--blacklist", nargs="+", default=["i", "input", "o", "output"], help="Blacklisted query parameters")
parser.add_argument("--args", nargs="+", default=[], help="binary parameters")
parser.add_argument("--content-type", nargs="+", default="application/json", help="binary parameters")

parsed_args, unknown_args = parser.parse_known_args()

# CONFIGURATION
# PORT = args.port
# BINARY_PATH = args.binary_path
# BINARY_ARGS = unknown_args
# BLACKLISTED_PARAMS = args.blacklist

class JSONWrapperHandler(http.server.BaseHTTPRequestHandler):
   
    def do_POST(self):
        
        # Parse query parameters
        parsed_url = urlparse(self.path)
        path= parsed_url.path
        query_params = parse_qs(parsed_url.query)
        
        # Check for blacklisted parameters
        for param in query_params:
            if param in  parsed_args.blacklist:
                error_msg =f"Parameter '{param}' is not allowed"
                self.send_error(400, error_msg)
                print(f"🌐 response:400 {error_msg}") 
                return

        request_args = []
        for k, v in query_params.items():            
            for val in v:
                request_args.append(k)
                if val:
                    request_args.append(val)

        # 1. READ THE REQUEST BODY
        content_length = int(self.headers.get('Content-Length', 0))
        if content_length == 0:
            error_msg ="Content-Length header missing"
            self.send_error(400, error_msg )
            print(f"🌐 response:400 {error_msg}") 
            return
            
        request_body = self.rfile.read(content_length)
        tokens=[]
        
        if parsed_args.exec:
            tokens+=[parsed_args.exec] 
        if path:
            if path!='/':
                current_dir = str(pathlib.Path.cwd())
                tokens+=[current_dir+path]
        tokens += [x for x in parsed_args.args if x] + request_args

        print( "🌐  command:"+(" ".join('"'+token+'"' for token in tokens)))
        print(f"🌐    stdin: {content_length} bytes\n")
        

        try:
            # 2. RUN THE BINARY
            # We pass the body to the binary's STDIN and capture STDOUT
            process = subprocess.Popen(
                tokens,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=sys.stdout,
                shell=False
            )
            
            # communicate() handles writing to stdin and reading from stdout safely
            stdout_data, stderr_data = process.communicate(input=request_body)

            std_err_len=0 if not stderr_data else len(stderr_data)
            std_out_len=0 if not stdout_data else len(stdout_data)
            

            # 3. SEND THE RESPONSE
            if process.returncode != 0:
                # binary returned error code, send 500 and  stderr+stdout
                self.send_response(500)
                self.send_header('Content-Length',std_err_len+std_out_len )
                self.end_headers()
              
                if stderr_data:
                    self.wfile.write(stderr_data)
                if stdout_data:
                    self.wfile.write(stdout_data)
                print(f"🌐 response:500 returncode was {process.returncode} sending {std_err_len+std_out_len} bytes")    
            else:
                # binary returned ok, send 200 and stdout
                self.send_response(200)
                self.send_header('Content-Type',parsed_args.content_type )
                self.send_header('Content-Length',std_out_len)
                self.end_headers()
                if stdout_data:
                    self.wfile.write(stdout_data)
                print(f"🌐 response:200 with {std_out_len} bytes")

        except Exception as e:
            # exception in server.py
            print(f"🌐 response:500 exception {str(e)}")
            self.send_error(500, f"server exception: {str(e)}")

if __name__ == "__main__":
    server_address = ('0.0.0.0', parsed_args.port)
    httpd = http.server.HTTPServer(server_address, JSONWrapperHandler)
    print(f"Serving HTTP  {parsed_args} ...")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server.")
