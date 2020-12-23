


f = open("names.txt")
print("{")
for line in f.readlines():
    no, name = line.split(",")    
    name = name.strip()
    no = int(no)
    print(r'''{%d, "%s"},'''%(no, name))
print("}")
