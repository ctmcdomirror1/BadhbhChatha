import ctypes
so_name='/usr/local/lib/libgmp.so' # or /usr/lib64/libgmp.so, etc
var_name='__gmp_version'
L=ctypes.cdll.LoadLibrary(so_name)
v=ctypes.c_char_p.in_dll(L,var_name)
print(v.value)

