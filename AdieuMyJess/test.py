import itertools as it

fr_cases = [[2, 2, 2, 1], [2, 2, 1, 1], [2, 2, 1, 0]]

cover_cases = [
    [2, 2, 2, 2],
    [2, 2, 2, 1],
    [2, 2, 2, 0],
    [2, 2, 1, 1],
    [2, 2, 1, 0],
    [2, 2, 0, 0],
    [2, 1, 1, 1],
    [2, 1, 1, 0],
    [2, 1, 0, 0],
    [2, 0, 0, 0],
    [1, 1, 1, 1],
    [1, 1, 1, 0],
    [1, 1, 0, 0],
    [1, 0, 0, 0],
    [0, 0, 0, 0],
]

def cost(p, fr_case):
   c = 0
   for i in range(4):
       if p[i] > fr_case[i]:
           c += p[i] - fr_case[i]
   return c


for i in range(3):
    catalogue = dict()
    for j in range(15):
        for p in it.permutations(cover_cases[j]):
            if p not in catalogue:
                catalogue[p] = [cost(p, fr_cases[i])]
            else:
                catalogue[p].append(cost(p, fr_cases[i]))
    print(catalogue)
