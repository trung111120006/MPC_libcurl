# Streaming Multiple Files

This repository demonstrates multiple file streaming using libcurl and HTTP/2. The project implements a client–server model where the client downloads and processes several media segments simultaneously from a remote server.

## Getting Started

### Step 1: Clone the source code

First, clone this repository to your local machine:

```bash
git clone https://github.com/trung111120006/MPC_libcurl
```

### Step 2: Create output directories

After cloning the project, create the required output folders inside the project directory:

```bash
mkdir decoded_output
mkdir download
```

These directories will be used to store decoded media files and downloaded segments.

### Step 3: Prepare the server

The server must be set up on a different device or machine. Clone the server repository using:

```bash
git clone https://github.com/trung111120006/server
```

> **Note:** In the server repository, the main file you need to pay attention to is `server.mpd`.

### Step 4: Run the server

Start the HTTP/2 server with the following command:

```bash
nghttpd -v -d . 8443 server.key server.crt
```

The server will run on port **8443**.  
Make sure that:

- The certificate files `server.key` and `server.crt` exist  
- Port 8443 is open and accessible from the client machine

### Step 5: Run the client

Go back to the client project and build as well as run the application:

```bash
make && make run
```

## Additional Information

- The client and server should run on different devices  
- Ensure that the MPD file path in the client matches the server configuration  
- You can simulate different bandwidth conditions on the server using:

```bash
tc qdisc
```

This helps evaluate streaming performance under various network scenarios.

## Requirements

- libcurl with HTTP/2 support  
- nghttp2 / nghttpd server  
- Valid TLS certificates  
- Linux environment recommended

---

Feel free to modify the MPD file and server configuration to test different streaming scenarios.
