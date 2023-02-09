io.write("Calling C...\n")

x, y = 32, 64

local ret = a_c_function("first", x, y, 123)


io.write("Function returned ", ret, "\n")
