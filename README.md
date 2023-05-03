# AVX
**AVX** is a simple language that only allows declaring variables with integer values and printing them but with using a code generator that transforms the AVX syntax to the [PyVM](https://github.com/unknown989/PyVM/) assembly language then running it.

# PyVM
**PyVM** is a runtime project I built with python to run a custom assembly syntax, it acts basically like JVM, 
[Check it out](https://github.com/unknown989/PyVM/)

# How to run
```ps
g++ -std=c++17 main.cpp -o avx
./avx file.avx
```

# Features
(Honestly nothing)
- Variable declaration
- Printing Variable


# TODO
- Add more types
- Add operations