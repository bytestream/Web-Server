all:
	cc -Wall -o server server.c misc.c

run:
	@echo "The server makes the assumption that when using POST it will only work with command line files that are executable."
	@echo "For example #/bin/bash or #/bin/perl at the top of the file"
	@echo "The allows for more languages to be used than building them into the web server exec code.."
	./server -v

test:
	@echo "I tested this using telnet and a web browser with varying syntaxes, for example:"
	@echo "\tGET fie.txt"
	@echo "\tGET /File.txt HTTP"
	@echo "\tGET /unknown-file.html HTTP/1.1"
	@echo "\tTests also included checking for the blank line in both POST and GET requests marking the end of HTTP headers."
	@echo "\tThis was particularly troublesome as web browsers tend to send all in one string whilst telnet is line by line."
	@echo "I also tested it using POST queries to perl files and checking the buffered output was correct."

clean:
	rm -rf server
