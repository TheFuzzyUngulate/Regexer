datatypes := types\stack\stack.c

run: $(datatypes) regexer.c
	gcc -g $(datatypes) regexer.c -o regexer

clean:
	del regexer.exe