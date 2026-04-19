# Source - https://stackoverflow.com/a/77512705
# Posted by Andrej Kesely
# Retrieved 2026-04-18, License - CC BY-SA 4.0

import http.server
import ssl

def get_ssl_context(certfile, keyfile):
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.minimum_version = ssl.TLSVersion.TLSv1_2
    context.maximum_version = ssl.TLSVersion.TLSv1_3
    context.load_cert_chain(certfile, keyfile)
    context.set_ciphers("@SECLEVEL=1:ALL")
    return context


class MyHandler(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers["Content-Length"])
        post_data = self.rfile.read(content_length)
        print(post_data.decode("utf-8"))


server_address = ("127.0.0.1", 5000)
httpd = http.server.HTTPServer(server_address, MyHandler)

context = get_ssl_context("localhost+1.pem", "localhost+1-key.pem")
httpd.socket = context.wrap_socket(httpd.socket, server_side=True)

httpd.serve_forever()

