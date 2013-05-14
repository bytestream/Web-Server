all:
	cc -Wall -o server server.c misc.c

run:
	./server -v

test:
	echo "I tested this using telnet and a web browser with varying syntaxes, for example:"
	echo "\tGET fie.txt"
	echo "\tGET /File.txt HTTP"
	echo "\tGET /unknown-file.html HTTP/1.1"
	echo "I also tested it using POST queries to perl files and checking the buffered output was correct."

clean:
	rm -rf server
