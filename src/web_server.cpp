#include "config.h"

// ----- Web Server --------------------------------------------------------------------

#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>

#include "cert/cert.h"
#include "cert/key.h"
#include "favicon.h"
#include "globals.h"

using namespace httpsserver;

// For the middleware
#include <functional>

// We define two new HTTP-Header names. Those headers will be used internally
// to store the user name and group after authentication. If the client provides
// these headers, they will be ignored to prevent authentication bypass.
#define HEADER_USERNAME "X-USERNAME"
#define HEADER_GROUP    "X-GROUP"

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(cert_der, cert_der_len, key_der, key_der_len);

// Create an SSL-enabled server that uses the certificate
// The contstructor takes some more parameters, but we go for default values here.
HTTPSServer secureServer = HTTPSServer(&cert);

#if 0
void handleRoot(HTTPRequest *req, HTTPResponse *res);
void handleFavicon(HTTPRequest *req, HTTPResponse *res);
void handle404(HTTPRequest *req, HTTPResponse *res);
#else
// Declare some handler functions for the various URLs on the server
void handleRoot(HTTPRequest *req, HTTPResponse *res);
void handleInternalPage(HTTPRequest *req, HTTPResponse *res);
void handleAdminPage(HTTPRequest *req, HTTPResponse *res);
void handlePublicPage(HTTPRequest *req, HTTPResponse *res);
void handle404(HTTPRequest *req, HTTPResponse *res);

// Declare a middleware function.
// Parameters:
// req: Request data, can be used to access URL, HTTP Method, Headers, ...
// res: Response data, can be used to access HTTP Status, Headers, ...
// next: This function is used to pass control down the chain. If you have done your work
//       with the request object, you may decide if you want to process the request.
//       If you do so, you call the next() function, and the next middleware function (if
//       there is any) or the actual requestHandler will be called.
//       If you want to skip the request, you do not call next, and set for example status
//       code 403 on the response to show that the user is not allowed to access a specific
//       resource.
//       For more details, see the definition below.
void middlewareAuthentication(HTTPRequest *req, HTTPResponse *res, std::function<void()> next);
void middlewareAuthorization(HTTPRequest *req, HTTPResponse *res, std::function<void()> next);
#endif

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
#if 0
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
               "<center></h1>404 Not Found</h1></center>"
               "<center><p>The requested resource was not found on this server.</p></center>"
               "</body>"
               "</html>");
}
#else
/**
 * The following middleware function is one of two functions dealing with access control. The
 * middlewareAuthentication() will interpret the HTTP Basic Auth header, check usernames and
 * password, and if they are valid, set the X-USERNAME and X-GROUP header.
 *
 * If they are invalid, the X-USERNAME and X-GROUP header will be unset. This is important because
 * otherwise the client may manipulate those internal headers.
 *
 * Having that done, further middleware functions and the request handler functions will be able to
 * just use req->getHeader("X-USERNAME") to find out if the user is logged in correctly.
 *
 * Furthermore, if the user supplies credentials and they are invalid, he will receive an 401
 * response without any other functions being called.
 */
void middlewareAuthentication(HTTPRequest *req, HTTPResponse *res, std::function<void()> next) {
  // Unset both headers to discard any value from the client
  // This prevents authentication bypass by a client that just sets X-USERNAME
  req->setHeader(HEADER_USERNAME, "");
  req->setHeader(HEADER_GROUP, "");

  // Get login information from request
  // If you use HTTP Basic Auth, you can retrieve the values from the request.
  // The return values will be empty strings if the user did not provide any data,
  // or if the format of the Authorization header is invalid (eg. no Basic Method
  // for Authorization, or an invalid Base64 token)
  std::string reqUsername = req->getBasicAuthUser();
  std::string reqPassword = req->getBasicAuthPassword();

  // If the user entered login information, we will check it
  if (reqUsername.length() > 0 && reqPassword.length() > 0) {

    // _Very_ simple hardcoded user database to check credentials and assign the group
    bool        authValid = true;
    std::string group     = "";
    if (reqUsername == ADMIN_USERNAME && reqPassword == ADMIN_PASSWORD) {
      group = "ADMIN";
    } else if (reqUsername == USER_USERNAME && reqPassword == USER_PASSWORD) {
      group = "USER";
    } else {
      authValid = false;
    }

    // If authentication was successful
    if (authValid) {
      // set custom headers and delegate control
      req->setHeader(HEADER_USERNAME, reqUsername);
      req->setHeader(HEADER_GROUP, group);

      // The user tried to authenticate and was successful
      // -> We proceed with this request.
      next();
    } else {
      // Display error page
      res->setStatusCode(401);
      res->setStatusText("Unauthorized");
      res->setHeader("Content-Type", "text/plain");

      // This should trigger the browser user/password dialog, and it will tell
      // the client how it can authenticate
      res->setHeader("WWW-Authenticate", "Basic realm=\"ESP32 privileged area\"");

      // Small error text on the response document. In a real-world scenario, you
      // shouldn't display the login information on this page, of course ;-)
      res->println("401. Unauthorized (try admin/secret or user/test)");

      // NO CALL TO next() here, as the authentication failed.
      // -> The code above did handle the request already.
    }
  } else {
    // No attempt to authenticate
    // -> Let the request pass through by calling next()
    next();
  }
}

/**
 * This function plays together with the middlewareAuthentication(). While the first function checks
 * the username/password combination and stores it in the request, this function makes use of this
 * information to allow or deny access.
 *
 * This example only prevents unauthorized access to every ResourceNode stored under an
 * /internal/... path.
 */
void middlewareAuthorization(HTTPRequest *req, HTTPResponse *res, std::function<void()> next) {
  // Get the username (if any)
  std::string username = req->getHeader(HEADER_USERNAME);

  // Check that only logged-in users may get to the internal area (All URLs starting with /internal)
  // Only a simple example, more complicated configuration is up to you.
  if (username == "" && req->getRequestString().substr(0, 9) == "/internal") {
    // Same as the deny-part in middlewareAuthentication()
    res->setStatusCode(401);
    res->setStatusText("Unauthorized");
    res->setHeader("Content-Type", "text/plain");
    res->setHeader("WWW-Authenticate", "Basic realm=\"ESP32 privileged area\"");
    res->println("401. Vous n'avez pas les autorisations requises.");

    // No call denies access to protected handler function.
  } else {
    // Everything else will be allowed, so we call next()
    next();
  }
}

// This is the internal page. It will greet the user with
// a personalized message and - if the user is in the ADMIN group -
// provide a link to the admin interface.
void handleInternalPage(HTTPRequest *req, HTTPResponse *res) {
  // Header
  res->setStatusCode(200);
  res->setStatusText("OK");
  res->setHeader("Content-Type", "text/html; charset=utf8");

  // Write page
  res->println("<!DOCTYPE html>"
               "<html>"
               "<head>"
               "<title>Internal Area</title><meta charset=\"utf-8\">"
               "</head>"
               "<body>");

  // Personalized greeting
  res->print("<h1>Hello ");
  // We can safely use the header value, this area is only accessible if it's
  // set (the middleware takes care of this)
  res->printStd(req->getHeader(HEADER_USERNAME));
  res->print("!</h1>");

  res->println("<p>Welcome to the internal area. Congratulations on successfully entering your "
               "password!</p>");

  // The "admin area" will only be shown if the correct group has been assigned in the
  // authenticationMiddleware
  if (req->getHeader(HEADER_GROUP) == "ADMIN") {
    res->println(
        "<div style=\"border:1px solid red;margin: 20px auto;padding:10px;background:#ff8080\">");
    res->println("<h2>You are an administrator</h2>");
    res->println("<p>You are allowed to access the admin page:</p>");
    res->println("<p><a href=\"/internal/admin\">Go to secret admin page</a></p>");
    res->println("</div>");
  }

  // Link to the root page
  res->println("<p><a href=\"/\">Go back home</a></p>");
  res->println("</body>");
  res->println("</html>");
}

void handleAdminPage(HTTPRequest *req, HTTPResponse *res) {
  // Headers
  res->setHeader("Content-Type", "text/html; charset=utf8");

  std::string header = "<!DOCTYPE html><html><head><title>Secret Admin Page</title><meta "
                       "charset=\"utf-8\"></head><body><h1>Secret Admin Page</h1>";
  std::string footer = "</body></html>";

  // Checking permissions can not only be done centrally in the middleware function but also in the
  // actual request handler. This would be handy if you provide an API with lists of resources, but
  // access rights are defined object-based.
  if (req->getHeader(HEADER_GROUP) == "ADMIN") {
    res->setStatusCode(200);
    res->setStatusText("OK");
    res->printStd(header);
    res->println(
        "<div style=\"border:1px solid red;margin: 20px auto;padding:10px;background:#ff8080\">");
    res->println("<h1>Congratulations</h1>");
    res->println("<p>You found the secret administrator page!</p>");
    res->println("<p><a href=\"/internal\">Go back</a></p>");
    res->println("</div>");
  } else {
    res->printStd(header);
    res->setStatusCode(403);
    res->setStatusText("Unauthorized");
    res->println("<p><strong>403 Unauthorized</strong> You have no power here!</p>");
  }

  res->printStd(footer);
}

// Just a simple page for demonstration, very similar to the root page.
void handlePublicPage(HTTPRequest *req, HTTPResponse *res) {
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title><meta charset=\"utf-8\"></head>");
  res->println("<body>");
  res->println("<h1>Hello World!</h1>");
  res->print("<p>Your server is running since ");
  char buff[20];
  sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d", startTime.tm_year + 1900, startTime.tm_mon,
          startTime.tm_mday, startTime.tm_hour, startTime.tm_min, startTime.tm_sec);
  res->println(".</p>");
  res->println("<p><a href=\"/\">Go back</a></p>");
  res->println("</body>");
  res->println("</html>");
}

// For details on the implementation of the hanlder functions, refer to the Static-Page example.
void handleRoot(HTTPRequest *req, HTTPResponse *res) {
  res->setHeader("Content-Type", "text/html");
  res->println(
      "<!DOCTYPE html>"
      "<html>"
      "<head><title>Camping Le Génévrier</title><meta charset=\"utf-8\"></head>"
      "<body>"
      "<center><h1>Hauteur Libre - Pont de la rivière La Mare!</h1>"
      "<p>Ce site est réservé pour le personnel du camping Le Génévrier.</p>"

      "<p><a href=\"/internal\">Gestion</a> | <a href=\"/public\">Hauteur Libre</a></p></center>"
      "</body>"
      "</html>");
}

void handle404(HTTPRequest *req, HTTPResponse *res) {
  req->discardRequestBody();
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html>"
               "<html>"
               "<head><title>Not Found</title><meta charset=\"utf-8\"></head>"
               "<body><center><h1>404 Not Found</h1><p>La resource demandée n'est pas disponible "
               "sur ce site.</p></center></body>"
               "</html>");
}

#endif

void webServerSetup() {

#if 0
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
#else
  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode *nodeRoot     = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode *nodeInternal = new ResourceNode("/internal", "GET", &handleInternalPage);
  ResourceNode *nodeAdmin    = new ResourceNode("/internal/admin", "GET", &handleAdminPage);
  ResourceNode *nodePublic   = new ResourceNode("/public", "GET", &handlePublicPage);
  ResourceNode *node404      = new ResourceNode("", "GET", &handle404);

  // Add the nodes to the server
  secureServer.registerNode(nodeRoot);
  secureServer.registerNode(nodeInternal);
  secureServer.registerNode(nodeAdmin);
  secureServer.registerNode(nodePublic);

  // Add the 404 not found node to the server.
  // The path is ignored for the default node.
  secureServer.setDefaultNode(node404);

  // Add the middleware. These functions will be called globally for every request
  // Note: The functions are called in the order they are added to the server.
  // This means, we need to add the authentication middleware first, because the
  // authorization middleware needs the headers that will be set by the authentication
  // middleware (First we check the identity, then we see what the user is allowed to do)
  secureServer.addMiddleware(&middlewareAuthentication);
  secureServer.addMiddleware(&middlewareAuthorization);
#endif

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
