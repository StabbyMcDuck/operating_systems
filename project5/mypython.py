# Regina Imhoff
# CS 344-400
# Project 5
# Python Crash Course

# Imports
import random
import string

# open up files
randomFile1 = open("random1", 'w')
randomFile2 = open("random2", 'w')
randomFile3 = open("random3", 'w')

files = [randomFile1, randomFile2, randomFile3]

for file in files:
    # generate random strings
    rand = ''.join(random.choice(string.ascii_lowercase) for n in xrange(10))

    # string written to files
    file.write(rand)
    file.write('\n') #new line
    file.close()

# open files to be read
randomFile1 = open("random1", 'r')
randomFile2 = open("random2", 'r')
randomFile3 = open("random3", 'r')

files = [randomFile1, randomFile2, randomFile3]

# print the letters into the files
for file in files:
    line = file.read(10) # 10 letters per line
    print line

# generate random numbers
from random import randint

# random int picked between 1 and 42 inclusive
randomNumber1 = randint(1,42)
randomNumber2 = randint(1,42)

# print out random numbers
print randomNumber1
print randomNumber2

# print the product of random numbers
product = randomNumber1 * randomNumber2
print product 
