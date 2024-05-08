datatypes := ..\types\list\lists.c

run: $(datatypes) testmake.c
	gcc -g $(datatypes) testmake.c -o testmake
	testmake

clean:
	del testmake.exe
	del tests.bat
	del *.txt