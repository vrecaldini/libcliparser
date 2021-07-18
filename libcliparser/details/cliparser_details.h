/**
 * @file cliparser_details.h
 * @brief implementation details for the cliparser library
 * @version 1.0
 * @date 2021-07-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef LIBCLIPARSER_DETAILS_CLIPARSER_DETAILS_H
#define LIBCLIPARSER_DETAILS_CLIPARSER_DETAILS_H
#include <string>
#include <concepts>

namespace cliparser {

    namespace details {

    /**
     * @brief CliParsableArgumentNoRefImplementation concept. If this concept is satisfied, the argument can be safely parsed from the command line input.
     * 
     * Only int, long, long long, bool, std::string, float, double, and long double satisfy this concept.
     * 
     * 
     * 
     * @tparam Argument a type
     */
    template <typename Argument>
    concept CliParsableArgumentNoRefImplementation = std::same_as<Argument, int> 
        || std::same_as<Argument, long> 
        || std::same_as<Argument, long long>
        || std::same_as<Argument, bool>
        || std::floating_point<Argument>
        || std::same_as<Argument, std::string>;
    
    }
}

#endif  // LIBCLIPARSER_DETAILS_CLIPARSER_DETAILS_H