# HTTP Server

This project is a simple HTTP server implemented in C with the help of the platform CodeCrafters. It handles basic HTTP GET and POST requests and can serve files from a specified directory. The server listens on port 4221 by default.

## Features

* Serve static files from a specified directory.
* Handle GET and POST requests.
* Echo responses for specific endpoints.
* Return User-Agent details.
* Allow file uploads via POST requests.

## Setup and Usage

### Compilation

Once you have cloned the repo and are in the project folder, you can compile the server with the following commands.

```bash
cd app
gcc server.c -o server
```

### Running the server

Run the compiled server with an optional --directory argument to specify the root directory for serving files:

```bash
./server --directory /path/to/directory
```

If no directory is specified, the server will use the current directory.

## Testing the Server

You can test the server using curl. Below are some examples:

1. Simple GET Request

```bash
curl -v http://localhost:4221/
```

2. Echo Request

```bash
curl -v http://localhost:4221/echo/hello
```

3. User-Agent Request
```bash
curl -v http://localhost:4221/user-agent
```

4. File Upload
```bash
curl -v -X POST http://localhost:4221/files/upload.txt -d 'Hello World'
```

5. File Download
```bash
curl -v http://localhost:4221/files/upload.txt
```

## How it works

### main()

The main function sets up the server, including the following steps:

1. Parse command-line arguments to get the directory to serve files from.
2. Change the working directory to the specified directory.
3. Set up the server socket to listen on port 4221.
4. Use setsockopt to set SO_REUSEPORT to avoid "Address already in use" errors.
5. Bind the socket to the address and port.
6. Listen for incoming connections.
7. Accept connections and handle them in a child process using fork.

### handle_connection()

The handle_connection function processes the client's request and sends an appropriate response:

1. Read the request using recv.
2. Parse the HTTP method and requested path.
3. Handle different paths:
  * /: Respond with "200 OK".
  * /echo/: Echo the rest of the path.
  * /user-agent: Return the User-Agent header.
  * /files/ with GET: Serve the requested file.
  * /files/ with POST: Save the uploaded content to the specified file.
4. Send the response using send.
