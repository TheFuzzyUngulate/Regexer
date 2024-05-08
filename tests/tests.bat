@echo off
setlocal EnableDelayedExpansion

echo compiling regexer.c
cd ..
make
echo compiled regexer.exe

echo testing t0.txt
regexer.exe tmp.c -f tests\t0.txt
gcc tmp.c -o tmp
set TEST="a"
tmp.exe !TEST!
IF ERRORLEVEL 0 (
	echo t0.txt passes testcase#1, as expected
	set TEST="b"
	tmp.exe !TEST!
	IF ERRORLEVEL 1 (
		echo t0.txt fails testcase#2, as expected
		set TEST="ab"
		tmp.exe !TEST!
		IF ERRORLEVEL 0 (
			echo t0.txt passes testcase#3, as expected
			set TEST="abab"
			tmp.exe !TEST!
			IF ERRORLEVEL 0 (
				echo t0.txt passes testcase#4, as expected
				set TEST="aaaaab"
				tmp.exe !TEST!
				IF ERRORLEVEL 0 (
					echo t0.txt passes testcase#5, as expected
				) ELSE (echo t0.txt unexpectedly fails on testcase#5)
			) ELSE (echo t0.txt unexpectedly fails on testcase#4)
		) ELSE (echo t0.txt unexpectedly fails on testcase#3)
	) ELSE (echo t0.txt unexpectedly passes on testcase#2)
) ELSE (echo t0.txt unexpectedly fails on testcase#1)
echo deleting temp files
del tmp.exe
del tmp.c

echo testing t1.txt
regexer.exe tmp.c -f tests\t1.txt
gcc tmp.c -o tmp
set TEST="0"
tmp.exe !TEST!
IF ERRORLEVEL 1 (
	echo t1.txt fails testcase#1, as expected
	set TEST="b"
	tmp.exe !TEST!
	IF ERRORLEVEL 1 (
		echo t1.txt fails testcase#2, as expected
		set TEST="0B"
		tmp.exe !TEST!
		IF ERRORLEVEL 1 (
			echo t1.txt fails testcase#3, as expected
			set TEST="0b0"
			tmp.exe !TEST!
			IF ERRORLEVEL 0 (
				echo t1.txt passes testcase#4, as expected
				set TEST="0B001"
				tmp.exe !TEST!
				IF ERRORLEVEL 0 (
					echo t1.txt passes testcase#5, as expected
					set TEST="0b2"
					tmp.exe !TEST!
					IF ERRORLEVEL 1 (
						echo t1.txt fails testcase#6, as expected
						set TEST="0B0101"
						tmp.exe !TEST!
						IF ERRORLEVEL 0 (
							echo t1.txt passes testcase#7, as expected
							set TEST="0B111"
							tmp.exe !TEST!
							IF ERRORLEVEL 0 (
								echo t1.txt passes testcase#8, as expected
							) ELSE (echo t1.txt unexpectedly fails on testcase#8)
						) ELSE (echo t1.txt unexpectedly fails on testcase#7)
					) ELSE (echo t1.txt unexpectedly passes on testcase#6)
				) ELSE (echo t1.txt unexpectedly fails on testcase#5)
			) ELSE (echo t1.txt unexpectedly fails on testcase#4)
		) ELSE (echo t1.txt unexpectedly passes on testcase#3)
	) ELSE (echo t1.txt unexpectedly passes on testcase#2)
) ELSE (echo t1.txt unexpectedly passes on testcase#1)
echo deleting temp files
del tmp.exe
del tmp.c

echo testing t2.txt
regexer.exe tmp.c -f tests\t2.txt
gcc tmp.c -o tmp
set TEST="\""
tmp.exe !TEST!
IF ERRORLEVEL 1 (
	echo t2.txt fails testcase#1, as expected
	set TEST="string"
	tmp.exe !TEST!
	IF ERRORLEVEL 1 (
		echo t2.txt fails testcase#2, as expected
		set TEST="\"\""
		tmp.exe !TEST!
		IF ERRORLEVEL 0 (
			echo t2.txt passes testcase#3, as expected
			set TEST="\"string\""
			tmp.exe !TEST!
			IF ERRORLEVEL 0 (
				echo t2.txt passes testcase#4, as expected
				set TEST="\"\"\""
				tmp.exe !TEST!
				IF ERRORLEVEL 0 (
					echo t2.txt passes testcase#5, as expected
					set TEST="\"friday afternoon\nyou already know\n\""
					tmp.exe !TEST!
					IF ERRORLEVEL 0 (
						echo t2.txt passes testcase#6, as expected
					) ELSE (echo t2.txt unexpectedly fails on testcase#6)
				) ELSE (echo t2.txt unexpectedly fails on testcase#5)
			) ELSE (echo t2.txt unexpectedly fails on testcase#4)
		) ELSE (echo t2.txt unexpectedly fails on testcase#3)
	) ELSE (echo t2.txt unexpectedly passes on testcase#2)
) ELSE (echo t2.txt unexpectedly passes on testcase#1)
echo deleting temp files
del tmp.exe
del tmp.c

echo testing t3.txt
regexer.exe tmp.c -f tests\t3.txt
gcc tmp.c -o tmp
set TEST="0"
tmp.exe !TEST!
IF ERRORLEVEL 1 (
	echo t3.txt fails testcase#1, as expected
	set TEST="x"
	tmp.exe !TEST!
	IF ERRORLEVEL 1 (
		echo t3.txt fails testcase#2, as expected
		set TEST="0x"
		tmp.exe !TEST!
		IF ERRORLEVEL 1 (
			echo t3.txt fails testcase#3, as expected
			set TEST="0x0"
			tmp.exe !TEST!
			IF ERRORLEVEL 0 (
				echo t3.txt passes testcase#4, as expected
				set TEST="0XA"
				tmp.exe !TEST!
				IF ERRORLEVEL 0 (
					echo t3.txt passes testcase#5, as expected
					set TEST="0xFg"
					tmp.exe !TEST!
					IF ERRORLEVEL 1 (
						echo t3.txt fails testcase#6, as expected
						set TEST="0XACC"
						tmp.exe !TEST!
						IF ERRORLEVEL 0 (
							echo t3.txt passes testcase#7, as expected
							set TEST="0X100"
							tmp.exe !TEST!
							IF ERRORLEVEL 0 (
								echo t3.txt passes testcase#8, as expected
							) ELSE (echo t3.txt unexpectedly fails on testcase#8)
						) ELSE (echo t3.txt unexpectedly fails on testcase#7)
					) ELSE (echo t3.txt unexpectedly passes on testcase#6)
				) ELSE (echo t3.txt unexpectedly fails on testcase#5)
			) ELSE (echo t3.txt unexpectedly fails on testcase#4)
		) ELSE (echo t3.txt unexpectedly passes on testcase#3)
	) ELSE (echo t3.txt unexpectedly passes on testcase#2)
) ELSE (echo t3.txt unexpectedly passes on testcase#1)
echo deleting temp files
del tmp.exe
del tmp.c

echo testing t4.txt
regexer.exe tmp.c -f tests\t4.txt
gcc tmp.c -o tmp
set TEST="cat"
tmp.exe !TEST!
IF ERRORLEVEL 0 (
	echo t4.txt passes testcase#1, as expected
	set TEST="dog"
	tmp.exe !TEST!
	IF ERRORLEVEL 0 (
		echo t4.txt passes testcase#2, as expected
		set TEST="at"
		tmp.exe !TEST!
		IF ERRORLEVEL 1 (
			echo t4.txt fails testcase#3, as expected
			set TEST="og"
			tmp.exe !TEST!
			IF ERRORLEVEL 1 (
				echo t4.txt fails testcase#4, as expected
			) ELSE (echo t4.txt unexpectedly passes on testcase#4)
		) ELSE (echo t4.txt unexpectedly passes on testcase#3)
	) ELSE (echo t4.txt unexpectedly fails on testcase#2)
) ELSE (echo t4.txt unexpectedly fails on testcase#1)
echo deleting temp files
del tmp.exe
del tmp.c

echo deleting regexer.exe
del regexer.exe
cd tests

endlocal & set FOO=%FOO%
@echo on
