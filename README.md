Web-Server
==========

During my time at the University of Miami whilst studying CSC524 - Computer Networks I was also asked to build a basic web server using pure C. The web server can be used to process GET and POST requests whereby GET requests will serve a static file and POST requests with the addition of POST data will execute the file as if it were CGI and pipe the output back to the client. Again for security reasons it only serves files in the current working directory, and will also only execute shell programs.
