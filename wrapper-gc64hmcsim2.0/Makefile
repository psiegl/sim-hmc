

default: lib/libhmcsim.so

bld:
	mkdir bld

extract: bld
	ar x lib/libhmcsim.a
	mv *.o bld/.

bld/hmc_sim.o: bld
	g++ -g -c -fPIC -o bld/hmc_wrapper.o src/hmc_wrapper.cpp
	#gcc -g -c -fPIC -o bld/hmc_sim.o src/hmc_sim.c

lib/libhmcsim.so: bld/hmc_sim.o extract
	g++ -Wl,--whole-archive -shared -Wl,-soname,-libhmcsim.so bld/*.o -o libhmcsim.so -Wl,--no-whole-archive -lsqlite3 -lz -ltcmalloc


test: bld
	gcc -L. src/test.c -lhmcsim -o bld/test.elf

clean:
	rm -rf bld lib/*.so
