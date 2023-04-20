#include "mbed.h"
#include "EthernetInterface.h"
#include "http_server.h"
#include "http_response_builder.h"

DigitalOut led(LED1);

#define STATIC_IP       0
#define IP_ADDRESS      "169.254.108.2"
#define NETWORK_MASK    "255.255.0.0"
#define GATEWAY_ADDRESS "169.254.108.1"

// Requests come in here
void request_handler(ParsedHttpRequest* request, TCPSocket* socket) {

    printf("Request came in: %s %s\n", http_method_str(request->get_method()), request->get_url().c_str());

    if (request->get_method() == HTTP_GET && request->get_url() == "/") {
        HttpResponseBuilder builder(200);
        builder.set_header("Content-Type", "text/html; charset=utf-8");

        char response[] = "<html><head><title>Hello from mbed</title></head>"
            "<body>"
                "<h1>mbed webserver</h1>"
                "<button id=\"toggle\">Toggle LED</button>"
                "<script>document.querySelector('#toggle').onclick = function() {"
                    "var x = new XMLHttpRequest(); x.open('POST', '/toggle'); x.send();"
                "}</script>"
            "</body></html>";

        builder.send(socket, response, sizeof(response) - 1);
    }
    else if (request->get_method() == HTTP_POST && request->get_url() == "/toggle") {
        printf("toggle LED called\n");
        led = !led;

        HttpResponseBuilder builder(200);
        builder.send(socket, NULL, 0);
    }
    else {
        HttpResponseBuilder builder(404);
        builder.send(socket, NULL, 0);
    }
}

int main() {

    printf("Mbed OS version is %d\n", MBED_VERSION);
    printf("This is a simple httpd example\n");

    printf("Connecting to network...\n");
    // Connect to the network (see mbed_app.json for the connectivity method used)
    NetworkInterface* network = NetworkInterface::get_default_instance();
    if (!network) {
        printf("Cannot connect to the network, see serial output\n");
        return 1;
    }

#if defined(STATIC_IP) && STATIC_IP
    network->set_network(IP_ADDRESS, NETWORK_MASK, GATEWAY_ADDRESS);
#else
    // Enalbe DHCP by default to obtain IP address
#endif

    network->connect();
    HttpServer server(network);

    nsapi_error_t res = server.start(8080, &request_handler);

    if (res == NSAPI_ERROR_OK) {
        SocketAddress sockaddr;
        res = network->get_ip_address(&sockaddr);
        if (res == NSAPI_ERROR_OK)
            printf("Server is listening at http://%s:8080\n", sockaddr.get_ip_address());
    }
    else {
        printf("Server could not be started... %d\n", res);
    }

    while(1) ThisThread::sleep_for(60s);
}