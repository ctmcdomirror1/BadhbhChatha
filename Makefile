build: 	
	gcc main.c \
		-I ./ ./chess.h ./position.h ./sanity.h ./sanity.c \
		-I ./dependencies/hungarian ./dependencies/hungarian/hungarian.h ./dependencies/hungarian/hungarian.c \
		-I ./position_filter ./position_filter/pawn_cost.h ./position_filter/pawn_cost.c ./position_filter/bishop_colour.h ./position_filter/bishop_colour.c ./position_filter/check.h ./position_filter/check.c ./position_filter/filter_common.h ./position_filter/filter_common.c \
		-I ./sample_tree ./sample_tree/create_tree.h ./sample_tree/create_tree.c ./sample_tree/tree_common.h ./sample_tree/search_tree.h ./sample_tree/search_tree.c \
		-I ./util ./util/util.h ./util/util.c  \
		-fPIC -lgmp -lm -mabm -mbmi -mbmi2 -o pos_gen -g

globals:
	gcc globals_test.c globals.h globals.c \
		-o globals_test -g

format:
	find . -name '*.h' ! -path './dependencies/*' -o -name '*.c' ! -path './dependencies/*' | xargs clang-format -i
