#include "config.h"

// ----- Web Server --------------------------------------------------------------------

#if OPTION_WEBSERVER

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>

#include "cert/cert.h"
#include "cert/key.h"
#include "favicon.h"

using namespace httpsserver;

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(cert_der, cert_der_len, key_der, key_der_len);

// Create an SSL-enabled server that uses the certificate
// The contstructor takes some more parameters, but we go for default values here.
HTTPSServer secureServer = HTTPSServer(&cert);

void handleRoot(HTTPRequest *req, HTTPResponse *res);
void handleFavicon(HTTPRequest *req, HTTPResponse *res);
void handle404(HTTPRequest *req, HTTPResponse *res);

#if 0

/* Style */
#define STYLE                                                                                      \
  "<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px "                 \
  "auto;font-size:15px}"                                                                           \
  "input{background:#f1f1f1;border:0;padding:0 "                                                   \
  "15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"                 \
  "#file-input{padding:0;border:1px solid "                                                        \
  "#ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"                            \
  "#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:"  \
  "0%;height:10px}"                                                                                \
  "form{background:#fff;max-width:258px;margin:75px "                                              \
  "auto;padding:30px;border-radius:5px;text-align:center}"                                         \
  ".btn{background:#3498db;color:#fff;cursor:pointer}</style>"

/* Login page */
String loginIndex = "<form name=loginForm>"
                    "<h1>ESP32 Login</h1>"
                    "<input name=userid placeholder='User ID'> "
                    "<input name=pwd placeholder=Password type=Password> "
                    "<input type=submit onclick=check(this.form) class=btn value=Login></form>"
                    "<script>"
                    "function check(form) {"
                    "if(form.userid.value=='" USERNAME "' && form.pwd.value=='" PASSWORD "')"
                    "{window.open('/serverIndex')}"
                    "else"
                    "{alert('Error Password or Username')}"
                    "}"
                    "</script>" STYLE;

/* Server Index Page */
String serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
    "<label id='file-input' for='file'>   Choose file...</label>"
    "<input type='submit' class=btn value='Update'>"
    "<br><br>"
    "<div id='prg'></div>"
    "<br><div id='prgbar'><div id='bar'></div></div><br></form>"
    "<script>"
    "function sub(obj){"
    "var fileName = obj.value.split('\\\\');"
    "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
    "};"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    "$.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "$('#bar').css('width',Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!') "
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>" STYLE;

void webServerSetup() {
  /*use mdns for host name resolution*/
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { // true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

#endif

void handleRoot(HTTPRequest *req, HTTPResponse *res) {
  // Status code is 200 OK by default.
  // We want to deliver a simple HTML page, so we send a corresponding content type:
  res->setHeader("Content-Type", "text/html");

  // The response implements the Print interface, so you can use it just like
  // you would write to Serial etc.
  res->println("<!DOCTYPE html>"
               "<html>"
               "<head><title>Hello World!</title></head>"
               "<body>"
               "<h1>Hello World!</h1>"
               "<p>Your server is running for ");
               
  // A bit of dynamic data: Show the uptime
  res->print((int)(millis() / 1000), DEC);

  res->println(" seconds.</p>"
               "</body>"
               "</html>");
}

void handleFavicon(HTTPRequest *req, HTTPResponse *res) {
  // Set Content-Type
  res->setHeader("Content-Type", "image/vnd.microsoft.icon");
  // Write data from header file
  res->write(FAVICON_DATA, FAVICON_LENGTH);
}

void handle404(HTTPRequest *req, HTTPResponse *res) {
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();

  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");

  // Set content type of the response
  res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>"
               "<html>"
               "<head>"
               "<title>Not Found</title>"
               "</head>"
               "<body>"
               "<h1>404 Not Found</h1>"
               "<p>The requested resource was not found on this server.</p>"
               "</body>"
               "</html>");
}

void webServerSetup() {

  ResourceNode *nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode *nodeFavicon = new ResourceNode("/favicon.ico", "GET", &handleFavicon);
  ResourceNode *node404     = new ResourceNode("", "GET", &handle404);

  // Add the root node to the server
  secureServer.registerNode(nodeRoot);

  // Add the favicon
  secureServer.registerNode(nodeFavicon);

  // Add the 404 not found node to the server.
  // The path is ignored for the default node.
  secureServer.setDefaultNode(node404);

  Serial.println("Starting server...");
  secureServer.start();
  if (secureServer.isRunning()) {
    Serial.println("Server ready.");
  }
}

void webServerTask(void *pvParameters) {
  if constexpr (WEBSERVER) {
    webServerSetup();
    for (;;) {
      secureServer.loop();
      delay(10);
    }
  }
}

#endif