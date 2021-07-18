/**
 * @file example1_check_if_path_exists.cpp
 * @brief simple program that checks whether a path p exists and prints the result n times
 * @version 0.1
 * @date 2021-07-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <string>
#include <filesystem>
#include <cstdlib>
#include <libcliparser/cliparser.h>

int main (int argc, char* argv[]) {
    // create a parser
    cliparser::CliParser parser(
        "checkpath", 
        "check if the path provided exists and print the result n times"
    );

    // add the options
    parser
        .option<std::string>("-p", "path")
        .option("-n", "times. Default value: 1", 1)
        .flag("--ignore-n", "ignore the -n flag and print the result 3 times.");
    
    // parse the input
    try {
        parser.parse(argc, argv);
    } 
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cout << std::endl << parser.help(true, false, true) << std::endl;
        std::exit(-1);
    }

    // get the options
    std::string p = parser.getOption<std::string>("-p");
    int n = parser.getOption<int>("-n");
    if (parser.getOption<bool>("--ignore-n")) {
        std::cout << "--ignore-n received. Setting n = 3\n";
        n = 3;
    }

    // additional step: add eventual constraits to your options
    if (n < 1) {
        std::cerr << "n must be strictly positive!\n";
        std::exit(-1);
    }
    
    // the program
    std::string result;
    if (std::filesystem::exists(std::filesystem::path(p))) result = p + " exists.\n";
    else result = p + " does not exist.\n";

    for (int i = 0; i < n; ++i) std::cout << result;
}