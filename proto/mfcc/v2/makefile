SRCS= $(wildcard *.c)
OBJS= $(wildcard o/*.o)
FLAGS= 
build: buildlibs
	gcc $(FLAGS) $(OBJS) -lm -o main
buildlibs:
	cd o && gcc $(FLAGS) -c ../*.c -lm
builwolibs:
	gcc $(FLAGS) *.c -lm -o main
echolibs:
	echo $(OBJS)
run: build
	./main
runwo: builwolibs
	./main
clean:
	rm o/*.o