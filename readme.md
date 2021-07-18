# libcliparser

This is a simple CLI parsing library for C++. Everything is defined under the `namespace` `cliparser`. In particular, there is a `cliparser::CliParsableArgument` `concept` and a `cliparser::CliParser` `class`.

---

## Table of Contents
- [libcliparser](#libcliparser)
  - [Table of Contents](#table-of-contents)
  - [Requirements](#requirements)
  - [Introduction](#introduction)
  - [Building libcliparser](#building-libcliparser)
    - [Building the docs](#building-the-docs)
  - [Additional Notes](#additional-notes)

---

## Requirements

- C++20

That's it.

## Introduction

`libcliparser` takes care of parsing the CLI input received by the `main` function for you. You  just have to set it up with a few lines of code:

- First, the `#include` directives:
    
    ```c++
    #include <libcliparser/cliparser.h>  // this contains the concept cliparser::CliParsableArgument and the class cliparser::CliParser, which represents the core of the library
    #include <libcliparser/exceptions.h>  // optional. Include this if you want to handle the exceptions depending on their type
    ```

- Secondly, you just have to create a `cliparser::CliParser` object. This may sound hard, but it is super easy (barely an inconvenience): just pass the name of your program and the description to the constructor. For example:

    ```c++
    cliparser::CliParser parser("my-app", "a description of my application");
    ```
- 
    Then, you should set all the options. If no value is provided to the parser, it assumes that it is a *required* option. Otherwise, the option is *optional*. Furthermore, if the option is *required*, the type of the option must be passed (that's right: `option` is a function template). If the option is *optional*, no type sshould be provided as its deduction from the provided default value should be left to the compiler in order to exploit *forwarding references*. In either case, only types that satisfy the `cliparser::CliParsableArgument` concept are allowed.
    Please note that the `cliparser::CliParser::option` method returns `cliparser::CliParser&`, therefore you can chain all your calls to this method:
    
    ```c++
    parser
        .option<bool>("-flag", "a required flag")
        .option("--usr", "guest", std::string("guest"))
        .option<int>("-n", "times")
        .option("--njobs", "number of jobs", 4);
    ```
    You may forgo the use of a `try` block around this part since it is complitely in your own hand, but if you want to feel safer, you could add a `try` block around this section and then catch the `cliparser::BadOptionFormatError` and the `cliparser::OptionRedefinitionError` exceptions while writing the first draft of your code and then either remove it or wrap it around `#ifndef NDEBUG` and `#endif` compiler directives. Passing the `-DNDEBUG` definition to the compiler should do the trick.

    - As you have probably noticed, the two exceptions mentioned earlier are just for the developers' sake. Indeed, they are defined only to prevent the use of the character `'='` in your options and to avoid the accidental redefinition of an option. A careful eye could probably catch both problems, but they may be a nice addition if this sequence of calls to `cliparser::CliParser::option` is done in an automated way (e.g. from an automatic tool of your own design, or from a datastructure (like an hash table)).

- 
    Now that you have defined the options your application should use, you may want to parse the arguments received from the command line. In most cases, it is recommended that this part be wrapped by a `try` block. 

    The `cliparser::CliParser::parse` method takes at least two parameters and up to four (counting two boolean flags):

    - an `int`: the argument counter (the number of arguments). Yes, it is exactly the first value received by the main function;

    - a `char**` (or `char* argv[]`): the arguments. The second argument received by main;

    - 
        a `bool`: `ignoreUnknownOptions`. By default, this is `false`. If an unknown option is passed to the program, the parser will throw a `cliparser::NoSuchOptionException` exception. If this flag is `true`, unknown options are silently ignored;
    
    - 
        a `bool`: `suppressMissingRequiredOptionsError`. By default, this is `false`. If some required options were not provided, the parser will throw a `cliparser::MissingRequiredOptionsError`.
    
        If this flag is `true`, the parser will not check whether any required option was set by the user. Please note that setting `suppressMissingRequiredOptionsError` to `true` is not recommended and should be avoided. Otherwise, you might find yourself forced to check whether each required option has been set by the user by calling `cliparser::CliParser::isOptionSetByUser` or by using a `try` block and catching the `cliparser::BadOptionAccessException`. Therefore, it might be best to ponder whether your required options are truly required.
    
    A program that does not ignore unknown options and requires all the required options to be set shoud behave roughly like this:
    ```c++
    try {
        parser.parse(argc, argv);
    }
    catch (const std::exception& e) {  // all the exceptions thrown by cliparser::CliParser are public children of std::exception and thus we can use a const lvalue reference to std::exception
        // handle your exception
    }
    ```
    or, if you want to handle the exceptions based on the type:
    ```c++
    try {
        parser.parse(argc, argv);
    }/
    catch (const cliparser::NoSuchOptionException& e) {
        // handle unknown argument
    }
    catch (const cliparser::MissingRequiredOptionsError& e) {
        // handle exceptions due to missing required options
    }
    catch (const std::invalid_argument& e) {
        // handle exceptions due to bad input
    }
    ```
    
- 
    Assuming that the input was parsed successfully, now you are free to fetch the values of your options by calling `cliparser::CliParser::getOption`, which is a function template that takes one template parameter: a type that satisfy the `cliparser::CliParsableArgument` concept.
    
    For this part, you may still use `try` block to detect `cliparser::BadOptionCastException`, `cliparser::BadOptionAccessException`, and `cliparser::NoSuchOptionException`. However, they are probably useful only when you are writing the first draft of your code or debugging since you know the option keys and types, and (assuming parse was called with `suppressMissingRequiredOptionsError=false`) all the required options have already been set at this point.

    Therefore, you may retrieve the options with:
    ```c++
    bool flag = parser.getOption<bool>("--flag");
    std::string usr = parser.getOption<std::string>("-usr");
    int n = parser.getOption<int>("-n");
    int njobs = parser.getOption<int>("-njobs");
    ```
- Now, you can do whatever you want.

---

## Building libcliparser

Building `libcliparser` is very easy. Just install your favourite compiler and cmake (> 3.16).
Then, `cd` to the root folder of the repository and

- linux:

        cmake . -B build -DCMAKE_BUILD_TYPE=Release; cmake --build build/ 

- windows:
    
        cmake . -B build; cmake --build build/ --config Release


for the release version

If you want the debug version, 

        cmake . -B build; cmake --build build/ 

<br />

> <span style="color:#ff4411">Please remember that a c++20 compiler is required.</span>

### Building the docs

To build the documentation for `libcliparser`, you need doxygen. Then `cd` to `libcliparser/docs`. Now, run the following command:

        doxygen

Done.

---

## Additional Notes

Right now, flags (such as `-h`) and aliases are not supported.