MNIST LeNet
===========

Detect Handwritten Digits using a LeNet trained with MNIST. This project comes with an simple HTML5 PWA which allows taking pictures and displaying the result.

Commandline Options
--------------------
(All of them are optional)
```
Allowed options:
  --help                Print help.
  --cert arg            Certificate file, if provided this file is used for tls
                        encryption of the websocket and http connections.
  --key arg             Private key for the certificate file, if provided this 
                        file is used for tls encryption of the websocket and 
                        http connections.
  --httpport arg        Port to listen for HTTP connections. (defaults to 8808)
  --wssport arg         Port to listen for WebSocket connections. (defaults to 
                        8809)
  --mnist arg           Path to folder which contains the mnist dataset. 
                        (defaults to ./mnist)
  --client arg          Path to folder which contains the HTML5 client. 
                        (defaults to ./client)
```

Quick Start
-----------

### LeNet & MNIST

The path specified by the --mnist argument needs to contain the mnist dataset which can be obtained from here: http://yann.lecun.com/exdb/mnist/

On first startup a LeNet will be trained and stored within the path specified by --mnist. This network will be used for all predictions.

A pre trained network is provided (mnist/mnist_network.dat)

### HTML5 client (PWA)

By default the PWA will only work with an valid SSL certificate (security restriction of most browsers).

To use the client without SSL certificate in chrome/chromium you need to enable the "Insecure origins treated as secure" setting for your host. To do so following steps need to be performed (this works on android aswell):

* Enter 'chrome://flags/' as url
* Search for "Insecure origins treated as secure"
* Mark setting as enabled and enter your host (including protocol and port, i.e. if programm runs on localhost: http://127.0.0.1:8808)

The PWA can then be accessed via the browser (on localhost via http://127.0.0.1:8808)

### Executables

All provided executables are completely statically linked and thus should be able to run on any device running Linux/Windows.

### Android

Because the executable is completely statically linked, it can be executed on rooted android phones. Download a package matching your CPU architecture and copy it to '/data' using a terminal emulator or adb.

### About
2020, Daniel Giritzer

https://page.nwrk.biz/giri
