build: 	
	gcc main.c \
		-I ./dependencies/hungarian ./dependencies/hungarian/hungarian.c \
		-I ./ \
		-I ./sample_tree ./sample_tree/piece_permutations.c \
		-I ./util \
		-fPIC -lgmp -lm -mabm -mbmi -mbmi2 -o pos_gen -g
		# -I ./position_filter ./position_filter/pawn_cost.c ./position_filter/bishop_colour.c ./position_filter/check.c ./position_filter/filter_common.c \

format:
	find . -name '*.h' ! -path './dependencies/*' -o -name '*.c' ! -path './dependencies/*' | xargs clang-format -i
