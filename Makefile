CC=g++ -O3
BIN_NAME=idock
OBJ_DIR=obj
BIN_DIR=../bin

$(BIN_DIR)/$(BIN_NAME): $(OBJ_DIR)/io_service_pool.o $(OBJ_DIR)/safe_counter.o $(OBJ_DIR)/array.o $(OBJ_DIR)/atom.o $(OBJ_DIR)/scoring_function.o $(OBJ_DIR)/receptor.o $(OBJ_DIR)/pka.o $(OBJ_DIR)/residue.o $(OBJ_DIR)/ligand.o $(OBJ_DIR)/result.o $(OBJ_DIR)/random_forest.o $(OBJ_DIR)/random_forest_x.o $(OBJ_DIR)/random_forest_y.o $(OBJ_DIR)/stopwatch.o $(OBJ_DIR)/main.o
	@mkdir -p $(BIN_DIR)
	${CC} -o $@ $^ -pthread -lstdc++fs -L${BOOST_ROOT}/lib -lboost_program_options

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(OBJ_DIR)
	${CC} -o $@ $< -c -std=c++17 -DNDEBUG -Wall -I${BOOST_ROOT}/include

clean:
	rm -f $(BIN_DIR)/$(BIN_NAME) $(OBJ_DIR)/*.o
	
setup:
	sudo cp $(BIN_DIR)/$(BIN_NAME) /usr/bin
