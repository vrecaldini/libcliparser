/**
 * @file exceptions.h
 * @brief exceptions that may be thrown by the cliparser::CliParser class.
 * @version 1.0
 * @date 2021-07-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef LIBCLIPARSER_EXCEPTION_H
#define LIBCLIPARSER_EXCEPTION_H
#include <exception>
#include <string>

namespace cliparser {

    /**
     * @brief This exception may be thrown when the program attempts to access an option that was not added to the CliParser
     * 
     * Public child of std::exception.
     * 
     */
    class NoSuchOptionException : public std::exception {
        public:

        /**
         * @brief overload of the what method
         * 
         * @return const char* the error message
         */
        const char* what() const noexcept override {return errorMsg.c_str();}

        /**
         * @brief Construct a new NoSuchOptionException object
         * 
         * @param option the option
         */
        NoSuchOptionException(const std::string& option) : errorMsg(std::string("Unrecognised option: ") + option) {}

        private:
        const std::string errorMsg;  ///< the error message
    };

    /**
     * @brief This exception may be thrown when a CliParser object attempts to redefine one of its options.
     * 
     * Public child of std::exception.
     * 
     */
    class OptionRedefinitionError : public std::exception {
        public:

        /**
         * @brief overload of the what method
         * 
         * @return const char* the error message
         */
        const char* what() const noexcept override {return errorMsg.c_str();}

        /**
         * @brief Construct a new OptionRedefinitionError object
         * 
         * @param option the option
         */
        OptionRedefinitionError(const std::string& option) : errorMsg(std::string("\033[1;31merror\033[0m: attempted to redefine the option \"") + option + std::string("\"")) {}

        private:
        const std::string errorMsg;  ///< the error message
    };

    /**
     * @brief This exception may be thrown when a CliParser object attempts to add an option that contains invalid characters (i.e. '=' or ' ').
     * 
     * Public child of std::exception.
     * 
     */
    class BadOptionFormatError : public std::exception {
        public:

        /**
         * @brief overload of the what method
         * 
         * @return const char* the error message
         */
        const char* what() const noexcept override {return errorMsg.c_str();}

        /**
         * @brief Construct a new BadOptionFormatError object
         * 
         * @param option the option
         */
        BadOptionFormatError(const std::string& option) : errorMsg(std::string("\033[1;31merror\033[0m: attempted to register an option with invalid characters. Option: \"") + option + std::string("\"")) {}

        private:
        const std::string errorMsg;  ///< the error message
    };

    /**
     * @brief This exception may be thrown when at least one required option was not passed to the CliParser
     * 
     * Public child of std::exception.
     * 
     */
    class MissingRequiredOptionsError : public std::exception {
        public:

        /**
         * @brief overload of the what method
         * 
         * @return const char* the error message
         */
        const char* what() const noexcept override {return errorMsg.c_str();}

        /**
         * @brief Construct a new MissingRequiredOption object
         * 
         * @param options a list of required options that have not been set. options.size() must not be 0
         */
        MissingRequiredOptionsError(const std::vector<std::string>& options) {
            static const std::string startMsg = "\033[1;31merror\033[0m: the option ";
            static const std::string endMsg = " is marked as required but no value was provided";
            errorMsg = "";

            for (std::vector<std::string>::size_type i = 0; i < options.size(); ++i) {
                errorMsg += startMsg + options[i] + endMsg;
                if (i != options.size()-1) errorMsg += "\n";
            }
            
        }

        private:
        std::string errorMsg;  ///< the error message
    };

    /**
     * @brief This exception may be thrown when the type of the option does not match the requested type
     * 
     * Public child of std::exception.
     * 
     */
    class BadOptionCastException : public std::exception {
        public:

        /**
         * @brief overload of the what method
         * 
         * @return const char* the error message
         */
        const char* what() const noexcept override {return errorMsg.c_str();}

        /**
         * @brief Construct a new BadOptionCastException object
         * 
         * @param option the option
         */
        BadOptionCastException(const std::string& option) : errorMsg(std::string("Wrong type for the option \"") + option + std::string("\"")) {}

        private:
        const std::string errorMsg;  ///< the error message
    };

    /**
     * @brief This exception may be thrown when the program attempts to access a REQUIRED option that was not set (i.e. it has no value)
     * 
     * Public child of std::exception.
     * 
     */
    class BadOptionAccessException : public std::exception {
        public:

        /**
         * @brief overload of the what method
         * 
         * @return const char* the error message
         */
        const char* what() const noexcept override {return errorMsg.c_str();}

        /**
         * @brief Construct a new BadOptionAccessException object
         * 
         * @param option the option
         */
        BadOptionAccessException(const std::string& option) : errorMsg(std::string("Bad option access. Option: \"") + option + std::string("\"")) {}

        private:
        const std::string errorMsg;  ///< the error message
    };

    
}

#endif  // LIBCLIPARSER_EXCEPTION_H