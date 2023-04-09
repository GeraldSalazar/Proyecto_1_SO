run:
	./Bin/Inicializador "buffer1" 21 5
	./Bin/Emisor "M" "buffer1" 21	
	./Bin/Receptor "Auto" "buffer1" 21	
	
compile:
	gcc -fsanitize=address Sources/Inicializador.c -o Bin/Inicializador -lrt -lc -lpthread
	gcc -fsanitize=address Sources/Emisor.c -o Bin/Emisor -lrt -lc -lpthread
	gcc Sources/Receptor.c -o Bin/Receptor -lrt -lc -lpthread
	gcc Sources/Finalizador.c -o Bin/Finalizador -lrt -lpthread

xd:
	./Bin/Receptor "Auto" "buffer1" 21	
	./Bin/Emisor "Auto" "buffer1" 21
	./Bin/Finalizador