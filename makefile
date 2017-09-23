all:	check
default:	check
clean:
		rm -rf worker 
worker: 
	gcc -o worker worker.c -lm

# checkpointer.o:checkpointer.c
# 		gcc -g -O0 -c  -Wall -Werror -fpic -o checkpointer.o checkpointer.c
# libckpt.so:checkpointer.o
# 			gcc -g -O0 -fno-stack-protector -shared -o libckpt.so checkpointer.o
# Hello.o:Hello.c
# 		gcc -g -O0 -c  -fno-stack-protector -Wall -Werror -fpic -o Hello.o Hello.c
# Hello:Hello.o
# 		gcc  -g -O0 -fno-stack-protector -o Hello Hello.o
# restart: myrestart.c
# 		gcc -g -O0 -g -fno-stack-protector -static -Wl,-Ttext-segment=5000000 -Wl,-Tdata=5100000 -Wl,-Tbss=5200000 -o restart myrestart.c
# myrestart: restart
# 	./restart myckpt