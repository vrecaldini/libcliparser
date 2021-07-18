/**
 * @file cliparser.h
 * @brief This is the main header of the library and defines the cliparser::CliParsableArgument concept and the cliparser::CliParser class.
 * @version 1.0
 * @date 2021-07-17
 * 
 * The cliparser::CliParsableArgument concept determines whether a type can be safely parsed from the CLI by this library. 
 * Right now, only int, long, long long, float, double, long double, bool and std::string are allowed.
 * 
 * cliparser::CliParser is a class that stores some app information and all the options (and their values) internally. 
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef LIBCLIPARSER_CLIPARSER_H
#define LIBCLIPARSER_CLIPARSER_H
#include <iostream>
#include <unordered_map>
#include <string>
#include <utility>
#include <concepts>
#include <vector>
#include <typeinfo>

#include <libcliparser/exceptions.h>  // cliparser exceptions
#include <libcliparser/details/cliparser_details.h>  // implementation details


/**
 * @brief namespace that holds anything defined in the cliparser library in order to avoid potential name collisions with other libraries 
 * 
 */
namespace cliparser {

    /**
     * @brief CliParsableArgument concept. If this concept is satisfied for a given type "Argument", a variable of that type can be safely parsed by CliParser
     * 
     * Only int, long, long long, bool, std::string, float, double, and long double (and their references) satisfy this concept.
     * 
     * PLEASE NOTE that the references are allowed in order to exploit forwarding references
     * 
     * @tparam Argument a type
     */
    template <typename Argument>
    concept CliParsableArgument = cliparser::details::CliParsableArgumentNoRefImplementation<typename std::decay<Argument>::type>;

    /**
     * @brief CliParser class. Simple CLI parsing. No positional arguments are allowed. Only one argument per option is allowed.
     * 
     * This class  stores some app information and all the options (and their values) internally. 
     * Objects of this class cannot be default, copy or move constructed, or copy or move assigned. 
     * 
     * 
     * Implementation details:
     * 
     * To achieve this result, CliParser defines an abstract base struct (OptionBase), which is publicly inherited by a template <CliParsableArgument Argument> Option class template. 
     * This allows the CliParser class to store and access options polymorphically by using std::unordered_map<std::string, OptionBase*>
     * 
     */
    class CliParser {
        public:
        /**
         * @brief user-defined destructor for the CliParser class
         * 
         */
        ~CliParser() {
            for (option_iterator it = cliOptions.begin(); it != cliOptions.end(); ++it) {
                delete it->second;
            }
        }

        /**
         * @brief Construct a new CliParser object
         * 
         * @param program the application name
         * @param description the description of the application
         */
        explicit CliParser(const std::string& program, const std::string& description) : appName(program), descr(description) {}

        /**
         * @brief Construct a new CliParser object
         * 
         * @param program the application name
         * @param description the description of the application (temporary object)
         */
        explicit CliParser(const std::string& program, std::string&& description) : appName(program), descr(std::move(description)) {}

        /**
         * @brief this function adds a required option opt to this CliParser object provided it has not already been defined, otherwise it throws an OptionRedefinitionError(opt).
         * No option should contain the character '=' or a white space. If it happens, BadOptionFormatError will be thrown
         * 
         * example:
         * 
         * CliParser parser("my app", "my descr");
         * parser.option<std::string>("-n", "name").option<std::string>("-s", "surname").option<std::string>("-u", "username");
         * 
         * 
         * @tparam Argument type or reference. The value held by the option will be of type typename std::decay<Argument>::type
         * @param opt option
         * @param descr description
         * @return CliParser& this object. Therefore, calls to this functions may be chained
         */
        template <CliParsableArgument Argument>
        CliParser& option(const std::string& opt, const std::string& descr);

        /**
         * @brief this function adds an optional option opt to this CliParser object provided it has not already been defined, otherwise it throws an OptionRedefinitionError(opt). 
         * If opt contains the character '=' or a white space, this function will throw BadOptionFormatError(opt)
         * example:
         * 
         * CliParser parser("my app", "my descr");
         * parser.option("-n", "name", std::string("Foo")).option("-s", "name", std::string("Bar")).option("-u", "username", std::string("FooBar01")).option("--year", "year", 2021);
         * 
         * 
         * @tparam Argument type or reference. This type should be deduced from the function parameter "defaultValue" to let the code use forwarding references. The value held by the option will be of type typename std::decay<Argument>::type. 
         * @param opt option
         * @param descr description
         * @param defaultValue the default Value
         * @return CliParser& this object. Therefore, calls to this functions may be chained
         */
        template <CliParsableArgument Argument>
        CliParser& option(const std::string& opt, const std::string& descr, Argument&& defaultValue);

        /**
         * @brief parse the input arguments. Here argc and argv should be the same parameters that the main function receives. argv[0] must be a string that represents the name used to invoke this program
         * 
         * This function may throw NoSuchOptionException, MissingRequiredOption
         * 
         * @param argc argument counter
         * @param argv argument value 
         * @param ignoreUnknownOptions if set to true, it will ignore unknown options. Otherwise, it will throw a NoSuchOptionException exception when unknown options are passed to the function. Default=false
         * @param suppressMissingRequiredOptionsError if set to true, it will not check whether any required option has been set by the user. Otherwise, it will throw a MissingRequiredOptionsError exception if a required option was not provided. Default=false
         */
        void parse(int argc, char* argv[], bool ignoreUnknownOptions=false, bool suppressMissingRequiredOptionsError=false);

        /**
         * @brief Get the opt option, if it exists and is available (e.g. it is OPTIONAL (default or set by the user) or REQUIRED and set by the user), otherwise throw NoSuchOptionException. 
         * Furthermore, if Argument does not match the option argument type, BadOptionCastException will be thrown. 
         * If the option is required, but its value has not been parsed by CliParser::parse yet, this function will throw a BadOptionAccessException exception.
         * 
         * @tparam Argument type. If typeid(Argument) matches that of the value held by the option, the value has type typename std::decay<Argument>::type
         * @param opt the option
         * @return std::decay<Argument>::type the value held by the option
         */
        template <CliParsableArgument Argument>
        [[nodiscard]] typename std::decay<Argument>::type getOption(const std::string& opt) const {
            const_option_iterator it = _getOptionIterator(opt);

            if (!it->second->good()) throw BadOptionAccessException(opt);
            if (it->second->type() != typeid(Argument)) throw BadOptionCastException(opt);
            // now we are certain that Argument and the type of the option are the same
            // therefore we can use static_cast
            return static_cast<Option<typename std::decay<Argument>::type>*>(it->second)->arg;
        }

        /**
         * @brief this function checks whether this CliParser object has the option identified by opt amongst its options
         * 
         * @param opt the option
         * @return true if option is included amongst all the other options
         * @return false otherwise 
         */
        [[nodiscard]] bool hasOption(const std::string& opt) const {return cliOptions.contains(opt);}

        /**
         * @brief this function checks whether the option identified by opt is optional. If option is not a valid option for this CliParser object, 
         * a NoSuchOptionException exception is thrown
         * 
         * @param opt the option
         * @return true if this option is optional
         * @return false otherwise
         */
        [[nodiscard]] bool isOptionOptional(const std::string& opt) const {
            return _getOptionIterator(opt)->second->isOptional();
        }

        /**
         * @brief this function checks whether the option identified by opt is set by the user. If option is not a valid option for this CliParser object, 
         * a NoSuchOptionException exception is thrown
         * 
         * @param opt the option
         * @return true if the option was set by the user
         * @return false otherwise
         */
        [[nodiscard]] bool isOptionSetByUser(const std::string& opt) const {
            return _getOptionIterator(opt)->second->isSetByUser();
        }

        /**
         * @brief Get the a vector that holds all the possible options
         * 
         * @return std::vector<std::string> a vector that holds all option names added to this CliParser object
         */
        std::vector<std::string> getAllPossibleOptions() const;

        /**
         * @brief get an help string
         * 
         * @param full flag that determines whether this function returns a brief help string or the full help string. Default: false
         * @param includeExecutablePath flag that determines whether this function should return the path to the executable in the help string. Default: false
         * @return std::string the output help string
         */
        [[nodiscard]] std::string help(bool full=false, bool includeExecutablePath=false) const;

        /**
         * @brief deleted default constructor
         * 
         */
        CliParser() = delete;

        /**
         * @brief deleted copy constructor
         * 
         */
        CliParser(const CliParser&) = delete;

        /**
         * @brief deleted move constructor
         * 
         */
        CliParser(CliParser&&) = delete;

        /**
         * @brief deleted copy assignment operator
         * 
         */
        CliParser& operator=(const CliParser&) = delete;

        /**
         * @brief deleted move assignment operator
         * 
         */
        CliParser& operator=(CliParser&&) = delete;

        private:

        struct OptionBase;  // forward declaration of the OptionBase struct

        /**
         * @brief parse input as an Argument
         * 
         * @tparam Argument the type/reference of the type of the argument. The new option will hold a value of type typename std::decay<Argument>::type. Argument satisfies the CliParsableArgument concept.
         * @param input the input const char*
         * @return Argument the parsed input
         */
        template <CliParsableArgument Argument>
        static Argument parseArg(const char* input);
        
        using option_dictionary = std::unordered_map<std::string, OptionBase*>;  ///< type that holds a dictionary to the options (i.e. an unordered_map that uses Key = std::string and value = OptionBase* )
        using option_iterator = typename option_dictionary::iterator;  ///< iterator from option_dictionary
        using const_option_iterator = typename option_dictionary::const_iterator;  ///< const iterator from option_dictionary
        using size_type = typename option_dictionary::size_type;  ///< size_type from option_dictionary
        
        /**
         * @brief OptionBase struct. OptionBase holds the base members to describe the metadata about an Option. Please note that this is an abstract class and thus cannot instantiate objects.
         *
         * This is the public base class of template <CliParsableArgument T> Option and will describe an interface and (when possible) its implementation. 
         * 
         */
        struct OptionBase {
            
            /**
            * @brief OPTION_INFO enum: information about an option
            * 
            */
            enum OPTION_INFO : char {
                BAD_OPTION = 0x00,  ///< bad option; this value should be used only to check the goodness of the option
                REQUIRED = 0x01, ///< the option is required
                OPTIONAL = 0x02,  ///< the option is optional
                SET_BY_USER = 0x04,  ///< the option was set by the user
                REQUIRED_PROVIDED_BY_USER = REQUIRED | SET_BY_USER,  ///< the option was flagged as REQUIRED and the user provided a value
                OPTIONAL_OVERRIDEN_BY_USER = OPTIONAL | SET_BY_USER  ///< the option was flagged as OPTIONAL and the user overrode its default value
            };

            std::string descr;  ///< description of this option
            OPTION_INFO info;  ///< information about this option

            /**
             * @brief Destroy the OptionBase object. Virtual destructor
             * 
             */
            virtual ~OptionBase()=default;

            /**
             * @brief Construct a new OptionBase object
             * 
             * @param str the description of the option
             * @param i information about the option
             */
            explicit OptionBase(const std::string& str, OPTION_INFO i) : descr(str), info(i) {}

            /**
             * @brief this function determines whether an option is "good" for use or not by casting the result of the bitwise and of info and OPTIONAL_OVERRIDEN_BY_USER to bool. 
             * In practice good() may return false only if info is REQUIRED or BAD_OPTION. 
             * Since BAD_OPTION is never set and only defined to define the goodness of an option, good() returns false only if info == REQUIRED, 
             * although the implementation still calculates info & OPTIONAL_OVERRIDEN_BY_USER
             *
             * NOTE: OPTIONAL_OVERRIDEN_BY_USER is defined as OPTIONAL | SET_BY_USER, which means that:
             * - if info IS REQUIRED but the user did not set it via command line, we have REQUIRED & (OPTIONAL | SET_BY_USER) = 0x01 & 0x06 = 0x00 == BAD_OPTION. Since BAD_OPTION is 0, it is treated as false when cast to bool 
             * - if info is REQUIRED_PROVIDED_BY_USER, we have (REQUIRED | SET_BY_USER) & (OPTIONAL | SET_BY_USER) = SET_BY_USER != BAD_OPTION. Hence, since SET_BY_USER is not 0, it is true
             * - if info is either OPTIONAL or OPTIONAL_OVERRIDEN_BY_USER, we have either OPTIONAL or OPTIONAL_OVERRIDEN_BY_USER. Neither is 0. Therefore, this is always true.
             * 
             *
             * This function is marked as virtual and final: no derived class can override it.
             *
             * @return true if the option can be used (e.g. it is OPTIONAL, REQUIRED_PROVIDED_BY_USER, or OPTIONAL_OVERRIDEN_BY_USER)
             * @return false otherwise
             */
            virtual bool good() const final {return static_cast<bool>(info & OPTIONAL_OVERRIDEN_BY_USER);}

            /**
             * @brief this function checks whether an option is optional
             *
             * NOTE: Virtual and final function. This function cannot be overriden.
             * 
             * @return true if this option is optional
             * @return false otherwise
             */
            virtual bool isOptional() const final {return static_cast<bool>(info & OPTIONAL);}

            /**
             * @brief this function checks whether an option has been set by the user. 
             * 
             * NOTE: Virtual and final function. This function cannot be overriden.
             * 
             * @return true if this option has been set by the user
             * @return false otherwise
             */
            virtual bool isSetByUser() const final {return static_cast<bool>(info & SET_BY_USER);}

            /**
             * @brief Get the typeid of the option held by the Option class. This function is pointless in OptionBase and it is marked as a pure virtual function. It must be implemented by the derived classes/structs that need to instantiate objects.
             * 
             * @return const std::type_info& the type id of the option (see Option<T>::type)
             */
            virtual const std::type_info& type() const = 0;

            /**
             * @brief set the value of the option from the input. Pure virtual function of OptionBase, since the value is stored in Option, its derived struct.
             * 
             * @param input string literal obtained from the command line argv
             */
            virtual void setArgFromInput(const char* input) = 0;
        };

        /**
         * @brief Derived from OptionBase, the Option struct contains all the member of its base struct and the argument value associated to the option. 
         * 
         * This struct cannot have any children as it is marked as final.
         * 
         * @tparam Argument any type that satisfies the CliParsableArgument concept
         */
        template <CliParsableArgument Argument> struct Option final : public OptionBase {

            Argument arg;  ///< the value associated to this option. std::optional<Argument> was not used because we can already establish whether the option is required, optional and set by the user
            
            /**
             * @brief Construct a new REQUIRED option
             * 
             * @param descr the option description
             */
            explicit Option(const std::string& descr) : OptionBase(descr, REQUIRED) {}

            /**
             * @brief Construct a new OPTIONAL option with a given defaultValue by copying defaultValue into arg
             * 
             * @param descr the description of the option
             * @param defaultValue the default value
             */
            explicit Option(const std::string& descr, const Argument& defaultValue) : OptionBase(descr, OPTIONAL), arg(defaultValue) {}

            /**
             * @brief Construct a new OPTIONAL object with a given defaultValue by moving defaultValue into arg
             * 
             * @param descr the description of the option
             * @param defaultValue the default value
             */
            explicit Option(const std::string& descr, Argument&& defaultValue) : OptionBase(descr, OPTIONAL), arg(std::move(defaultValue)) {}

            /**
             * @brief return typeid(Argument), where Argument is the type of the value "arg" stored in this option object
             * 
             * @return const std::type_info& typeid(Argument)
             */
            const std::type_info& type() const final override {return typeid(Argument);}

            /**
             * @brief set arg to the result of the parsing of the input string literal
             * 
             * Example: Argument = double and input = "3.1415". arg = 3.1415
             * 
             * @param input input obtained from the command line argv
             */
            void setArgFromInput(const char* input) final override {
                arg = parseArg<Argument>(input);
                info = static_cast<OPTION_INFO>(info | SET_BY_USER);
            }
        };

        /**
         * @brief get the iterator to the option whose key is opt from the option_dictionary (aka std::unordered_map<std::string, OptionBase*>). If opt is not a key of the dictionary, throw NoSuchOptionException.
         * 
         * @param opt the option key
         * @return const_option_iterator the const iterator to the option
         */
        const_option_iterator _getOptionIterator(const std::string& opt) const {
            const_option_iterator it = cliOptions.find(opt);
            if (it == cliOptions.end()) throw NoSuchOptionException(opt);
            return it;
        }

        /**
         * @brief get the iterator to the option whose key is opt from the option_dictionary (aka std::unordered_map<std::string, OptionBase*>). If opt is not a key of the dictionary, throw NoSuchOptionException.
         * 
         * non-const overload
         * @param opt the option key
         * @return option_iterator the const iterator to the option
         */
        option_iterator _getOptionIterator(const std::string& opt) {
            option_iterator it = cliOptions.find(opt);
            if (it == cliOptions.end()) throw NoSuchOptionException(opt);
            return it;
        }



        std::string appName;  ///< name of the application
        std::string executablePath;  ///< path of the executable file
        std::string descr;  ///< description of the application
        option_dictionary cliOptions;  ///< dictionary of options

    };

}



////////////////////////////
///// IMPLEMENTATION //////
//////////////////////////

namespace cliparser {

    template <CliParsableArgument Argument>
    CliParser& CliParser::option(const std::string& opt, const std::string& descr) {
        if (hasOption(opt)) throw OptionRedefinitionError(opt);

        if (std::string::size_type pos = opt.find_first_of("= "); pos != std::string::npos) throw BadOptionFormatError(opt);

        cliOptions.insert_or_assign(opt, new Option<typename std::decay<Argument>::type>(descr));
        
        return *this;
    }

    template <CliParsableArgument Argument>
    CliParser& CliParser::option(const std::string& opt, const std::string& descr, Argument&& defaultValue) {
        if(hasOption(opt)) throw OptionRedefinitionError(opt);

        if (std::string::size_type pos = opt.find_first_of("= "); pos != std::string::npos) throw BadOptionFormatError(opt);

        cliOptions.insert_or_assign(opt, new Option<typename std::decay<Argument>::type>(descr, std::forward<Argument>(defaultValue)));

        return *this;
    } 

    /**
     * 
     * Please note that each CliParser::parseArg template specialisation could either be declared here and defined in a .cpp file or defined here as inline.
     * Those that appear small enough will be defined here as inline
     * 
     *  
     */

    /**
     * @brief specialisation of CliParser::parseArg with Argument = int. This function calls std::stoi
     * 
     * @param input the input
     * @return int the parsed integer
     */
    template <> inline int CliParser::parseArg<int>(const char* input) {
        return std::stoi(input);
    }
    
    /**
     * @brief specialisation of CliParser::parseArg with Argument = long. This function calls std::stol
     * 
     * @param input the input
     * @return long the parsed integer
     */
    template <> inline long CliParser::parseArg<long>(const char* input) {
        return std::stol(input);
    }

    /**
     * @brief specialisation of CliParser::parseArg with Argument = long long. This function calls std::stoll
     * 
     * @param input the input
     * @return long long the parsed integer
     */
    template <> inline long long CliParser::parseArg<long long>(const char* input) {
        return std::stoll(input);
    }

    /**
     * @brief specialisation of CliParser::parseArg with Argument = float. This function calls std::stof
     * 
     * @param input the input
     * @return float the parsed integer
     */
    template <> inline float CliParser::parseArg<float>(const char* input) {
        return std::stof(input);
    }

    /**
     * @brief specialisation of CliParser::parseArg with Argument = double. This function calls std::stod
     * 
     * @param input the input
     * @return double the parsed integer
     */
    template <> inline double CliParser::parseArg<double>(const char* input) {
        return std::stod(input);
    }

    /**
     * @brief specialisation of CliParser::parseArg with Argument = long double. This function calls std::stold
     * 
     * @param input the input
     * @return long double the parsed integer
     */
    template <> inline long double CliParser::parseArg<long double>(const char* input) {
        return std::stold(input);
    }

    /**
     * @brief specialisation of CliParser::parseArg with Argument = std::string. This function calls the string constructor
     * 
     * @param input the input
     * @return std::string the parsed integer
     */
    template <> inline std::string CliParser::parseArg<std::string>(const char* input) {
        return std::string(input);
    }

    /**
     * @brief specialisation of CliParser::parseArg with Argument = bool. 
     * 
     * This function copies the input, converts it to lowercase and compares it with "y", "true", "n" and "false". If no comparison returns true, the input is rejected and std::invalid_argument is thrown 
     * 
     * @param input the input
     * @return true if the input is either "y" or "true" (ignoring the case)
     * @return false otherwise if the input is either "n" or "false" (ignoring the case)
     */
    template <> bool CliParser::parseArg<bool>(const char* input);

}

#endif  // LIBCLIPARSER_CLIPARSER_H