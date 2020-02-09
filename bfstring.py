import sys

#This is a very quick and dirty script to output bf text

def makeChar(c):
    c = ord(c)
    tens = c / 10
    ones = c % 10
    s = ">++++++++++"
    s += "[-<"

    for i in xrange(tens):
        s += "+"
    s += ">]<"

    for i in xrange(ones):
        s += "+"

    return s


file = open(sys.argv[1])
s = file.read()
file.close()



output = "->"
for i, c in enumerate(s):
    added_stuff = makeChar(c)
    output += added_stuff
    output += ">"

output += "+[-<+]->[.>]"

actual_output = ""
for i, c in enumerate(output):
    if (i % 100) == 0 and i != 0:
        actual_output += '\n'
    actual_output += c

outFileName = sys.argv[1].split('.')[0]

file = open(outFileName + ".bf", 'w+')
file.write(actual_output)
file.close()



