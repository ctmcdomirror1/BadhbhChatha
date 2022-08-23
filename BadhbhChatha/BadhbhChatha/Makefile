build: 	
	gcc main.c \
		-I ./ ./sanity.c \
		-I ./dependencies/hungarian ./dependencies/hungarian/hungarian.c \
		-I ./sample_tree ./sample_tree/create_tree.c ./sample_tree/search_tree.c \
		-I ./util ./util/util.c  \
		-fPIC -lgmp -lm -mabm -mbmi -mbmi2 -o pos_gen -g
		# -I ./position_filter ./position_filter/pawn_cost.h ./position_filter/pawn_cost.c ./position_filter/bishop_colour.h ./position_filter/bishop_colour.c ./position_filter/check.h ./position_filter/check.c ./position_filter/filter_common.h ./position_filter/filter_common.c \

format:
	find . -name '*.h' ! -path './dependencies/*' -o -name '*.c' ! -path './dependencies/*' | xargs clang-format -i
