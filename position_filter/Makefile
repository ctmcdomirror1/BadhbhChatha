check:
	gcc check.c check.h check_test.c \
		-I ./ ./filter_common.h ./filter_common.c \
		-I ../ ../position.h ../chess.h \
		-I ../util ../util/util.c ../util/util.h \
		-mabm -mbmi -lm -fvar-tracking -o check_test -g

cost:
	gcc pawn_cost.c pawn_cost.h pawn_cost_test.c filter_common.h filter_common.c \
		-I ../dependencies/hungarian ../dependencies/hungarian/hungarian.c ../dependencies/hungarian/hungarian.h \
		-I ../util ../util/util.c ../util/util.h \
		-I ../ ../chess.h ../position.h \
		-mabm -mbmi -fvar-tracking -Wall -o cost_test -g

common:
	gcc filter_common_test.c filter_common.h filter_common.c \
		-o filter_test -g
