# Lispy

Project from the website [Build your own Lisp](http://www.buildyourownlisp.com/).

## What is Lispy ?

Lispy is a small Lisp-like programming language written in C, using [mpc](https://github.com/orangeduck/mpc) library to parse the input easily.

## What are Lispy's features ?

Lispy supports all the basic features that you would expect from a programming language, like :

- Variables
- Functions
- Conditionals
- Error handling
- Standard library
- etc.

But, there is more to it :

- S-Expression : **symbolic-expression** is a list of one or more expressions (number, string, symbol, etc.).
- Q-Expression : **quoted-expression** is a list of one or more expressions that are not evaluated during process but instead left exactly as they were typed in.

## Examples of programs written in Lispy

Hello World! :

```lisp
("Hello World!")
```

Fibonacci function :

```lisp
(fun {fib n} {
  select
    { (== n 0) 0 }
    { (== n 1) 1 }
    { otherwise (+ (fib (- n 1)) (fib (- n 2))) }
})
```

Factorial function :

```lisp
(fun {fac n} {
   select
      { (== n 0) 1 }
      { otherwise (* (n) (fac(- n 1)))}
})
```
