import http.server
import os
import sys

import requests


class CookooClockHandler(http.server.BaseHTTPRequestHandler):

    captured = False
    
    def do_GET(self):
        print("Hello!")
        response = requests.get('http://helios:8000/module/status/json')
        data = response.json()
        self.captured = data['status']['controllingFaction'] != '0'
        if self.captured:
            os.system("aplay /usr/share/skype/sounds/TransferRequest.wav")
        else:
            os.system("aplay /usr/share/skype/sounds/CallHangup.wav")


def main(HandlerClass=CookooClockHandler,
         ServerClass=http.server.HTTPServer, protocol="HTTP/1.0", port=9000, bind=""):
    server_address = (bind, port)
    with ServerClass(server_address, HandlerClass) as httpd:
        sa = httpd.socket.getsockname()
        serve_message = "Serving HTTP on {host} port {port} (http://{host}:{port}/) ..."
        
        print(serve_message.format(host=sa[0], port=sa[1]))
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nKeyboard interrupt received, exiting.")
            sys.exit(0) 
    

if __name__ == '__main__':
    main()
    
