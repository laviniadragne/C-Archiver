build: archiver

star_dust: archiver.c
	gcc archiver.c -o archiver -Wall -Wextra

clean:
	rm -f archiver


