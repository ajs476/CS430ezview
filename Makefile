all:
	gcc -I/usr/local/include -L/usr/local/lib -lglfw -framework OpenGL -framework Cocoa -framework CoreVideo -framework IOKit ezview.c -o ezview

clean:
	rm -rf ezview *~
