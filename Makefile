all:cpu_load_gen

cpu_load_gen: cpu_load_gen.c cpu_usage.c
	$(CC) -o $@  $^ -lpthread -lm 

clean: 
	rm -rf *.o cpu_load_gen
