CC=g++ -O2

bin/idock: obj/io_service_pool.o obj/safe_counter.o obj/array.o obj/string.o obj/atom.o obj/scoring_function.o obj/receptor.o obj/residue.o obj/ligand.o obj/result.o obj/random_forest.o obj/random_forest_x.o obj/random_forest_y.o obj/stopwatch.o obj/main.o
	${CC} -o $@ $^ -pthread -L${BOOST_ROOT}/stage/linux_x64/lib -lstdc++fs -lboost_program_options-gcc8-mt-s-x64-1_69 -lboost_thread-gcc8-mt-s-x64-1_69

obj/%.o: src/%.cpp
	${CC} -o $@ $< -c -std=c++17 -Wall -Wno-reorder -I${BOOST_ROOT}

clean:
	rm -f bin/idock obj/*.o
