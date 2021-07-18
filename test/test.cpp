#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <exception>
#include <cassert>
#include <cstdlib>
#include <libcliparser/cliparser.h>
#include <libcliparser/exceptions.h>
int main (int argc, char* argv[]) {

    cliparser::CliParser parser("test", "this is a test program for the cliparser library.");

    // not using try-catch because all the options are explicitly set, unique and correctly formatted
    parser
        .option<int>("-n", "integer")
        .option<double>("-d", "double")
        .option<bool>("-b", "bool")
        .option("--flag", "an optional flag", false)  
        .option("-q", "optional float", 3.22f)
        .option<std::string>("-f", "file")
        .flag("--help", "print help and ignore everything else");

    
    // some tests with assert; if compiled with -DNDEBUG, these will do nothing
    assert(parser.isOptionOptional("-n") == false);
    assert(parser.isOptionOptional("-d") == false);
    assert(parser.isOptionOptional("-b") == false);
    assert(parser.isOptionOptional("--flag") == true);
    assert(parser.getOption<bool>("--flag") == false);
    assert(parser.isOptionOptional("-q") == true);
    assert(parser.getOption<float>("-q") == 3.22f);
    assert(parser.isOptionOptional("-f") == false);
    assert(parser.isOptionFlag("--help"));
    assert(parser.isOptionSetByUser("--help") == false);

    std::vector<std::string> optionVec = parser.getAllPossibleOptions();
    std::cout << "options: ";
    for (const decltype(optionVec)::value_type& opt : optionVec) {
        std::cout << opt << "\t";
    }
    std::cout << std::endl;

    #ifndef NDEBUG
    std::cout << "\n\nStarting tests...\n\n";
    // test cliparser::BadOptionFormatError
    {
        std::cout << "Testing cliparser::BadOptionFormatError...\n";
        std::string badOption = "-q=fs3s";
        bool g = false;
        try {
            parser.option<std::string>(badOption, "bad option");
        }
        catch (const cliparser::BadOptionFormatError& e) {
            std::cerr << e.what() << std::endl;
            g = true;
        }
        catch (...) {
            // code
        }

        if (!g) std::cout << "Test failed.\n";
        else std::cout << "Test passed.\n";
        assert(g);
    }
    // test on cliparser::OptionRedefinitionError
    {
        // this test must break if "-n" has not been defined yet
        assert(parser.hasOption("-n"));

        std::cout << "Testing cliparser::OptionRedefinitionError...\n";

        try {
            parser.option<int>("-n", "redefinition of integer n");
            assert(false); // no exception: test failed
        }
        catch(const cliparser::OptionRedefinitionError& e) {
            std::cerr << e.what() << std::endl;
        }
        catch(...) {
            std::cerr << "test failed: wrong exception\n";
            assert(false); // wrong exception
        }
        std::cout << "Test passed.\n";
    }

    // test on cliparser::BadOptionAccessException
    {
        // this test must break if "-n" has not been defined yet
        assert(parser.hasOption("-n"));

        std::cout << "Testing cliparser::BadOptionAccessException...\n";

        try {
            int n = parser.getOption<int>("-n");
            assert(false); // no exception: test failed
        }
        catch(const cliparser::BadOptionAccessException& e) {
            std::cerr << e.what() << std::endl;
        }
        catch(...) {
            std::cerr << "test failed: wrong exception\n";
            assert(false); // wrong exception
        }
        std::cout << "Test passed.\n";
    }

    // a test on cliparser::BadOptionCastException
    {
        assert(parser.hasOption("--flag"));  // using --flag since it is optional and therefore it will never throw BadOptionAccessException when accessed via parser.getOption

        
        std::cout << "Testing cliparser::BadOptionCastException...\n";

        bool hasExceptionHappened = false;
        try {
            // "--flag" is boolean
            std::cout << "Attempting to get the bool option \"--flag\" as a long double...\n";
            double n = parser.getOption<long double>("--flag"); 
        }
        catch (const cliparser::BadOptionCastException& e) {
            std::cerr << e.what() << std::endl;
            hasExceptionHappened = true;
        }
        catch (...) {
            // code
        }
        if (!hasExceptionHappened) std::cout << "Test failed.\n";
        assert(hasExceptionHappened == true);
        std::cout << "Test passed.\n";
    }

    // a test on cliparser::NoSuchOptionException
    {
        std::string missingOpt = "-qqqqqqqqqqqqqqqqqqqq";
        // this test is not valid if parser has the missingOpt option
        assert(!parser.hasOption(missingOpt));
        std::cout << "Testing cliparser::NoSuchOptionException...\n";
        bool hasExceptionHappened = false;
        try {
            bool res = parser.isOptionOptional(missingOpt);
        } catch (const cliparser::NoSuchOptionException& e) {
            std::cerr << e.what() << std::endl;
            hasExceptionHappened = true;
        }
        if (!hasExceptionHappened) std::cout << "Test failed.\n";
        assert(hasExceptionHappened == true);
        std::cout << "Test passed.\n";
    }

    std::cout << "\n\nReached the end of the test section\n\n";
    #endif

    try {
        /**
         * default parameters ignoreUnknownOptions=false and suppressMissingRequiredOptionsError=false
         * Therefore, this function may throw, depending on the cli arguments
         */
        parser.parse(argc, argv);  
    } 
    catch (const std::exception& e) { // this catches all the exceptions thrown by parse
        if (parser.getOption<bool>("--help")) {
            std::cout << parser.help(true, false, true) << std::endl;
            return 0;
        }
        else {
            std::cerr << e.what() << std::endl;
            std::exit(-1);
        }
    }
    // we could have always used:
    // catch (const cliparser::NoSuchOptionException& e) {
    //     // handle specific exception
    //     std::cerr << e.what() << std::endl;
    // }
    // catch (const cliparser::MissingRequiredOptionsError& e) {
    //     // handle specific exception
    //     std::cerr << e.what() << std::endl;
    // }
    // catch (const std::invalid_argument& e) {
    //     // handle specific exception
    //     std::cerr << e.what() << std::endl;
    // }

    
    // now reading. 
    /*
        Try blocks have been omitted here since they are an additional layer of security. 
        If you are CERTAIN that the options and types are correct (and the developer usually is, since he/she usually sets the options and types at the beginning of the program),
        and if you have either let parse check whether there are any missing required options, then you may skip the try blocks at your own risk.
    */

    if (parser.getOption<bool>("--help")) {
        std::cout << "The CLI input was parsed successfully. Since --help was received, everyting else will be ignored.\n"
            << parser.help(true, false, true) << std::endl;
        return 0;
    }
    int n = parser.getOption<int>("-n");
    double d = parser.getOption<double>("-d");
    bool b = parser.getOption<bool>("-b"), flag = parser.getOption<bool>("--flag");
    float q = parser.getOption<float>("-q");
    std::string f = parser.getOption<std::string>("-f");

    std::cout 
        << std::boolalpha 
        <<"\n\nPassed args: " << std::endl
        << "\t-n: " << n << "\n\t-d: " << d << "\n\t-b: " << b
        << "\n\t--flag: " << flag << "\n\t-q: " << q << "\n\t-f: " << f 
        << std::endl;

    if (!parser.isOptionSetByUser("--flag") && !parser.isOptionSetByUser("-q")) {
        std::cout << "\n\nNeither --flag nor -q was passed to the program, "
            "therefore they kept their default value.\n\n";
    }
    else {
        std::cout << std::endl;
        
        if (parser.isOptionSetByUser("--flag")) std::cout << "The default value of the option '--flag' was overriden by the user\n";
        if (parser.isOptionSetByUser("-q")) std::cout << "The default value of the option '-q' was overriden by the user\n";
    }

    std::filesystem::path p(f);
    if (std::filesystem::is_regular_file(p)) {
        std::cout << "\n" << p << " exists and is a regular file.\n";
    }
    else {
        if (!std::filesystem::exists(p)) std::cout << p << " does not exist.\n";
        else std::cout << p << " exists but is not a regular file.\n";
    }

}
