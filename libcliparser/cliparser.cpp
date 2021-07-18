#include <string>
#include <vector>
#include <string_view>
#include <exception>

#include <libcliparser/cliparser.h>
#include <libcliparser/exceptions.h>

namespace cliparser {

    void CliParser::parse(int argc, char* argv[], bool ignoreUnknownOptions, bool suppressMissingRequiredOptionsError) { 
        if (argc == 0) return;  // handle corner case: argc == 0. If this is the case, do nothing
        executablePath = argv[0];
        int i = 1;
        while (i < argc) {

            std::string_view view(argv[i]);

            if (std::string_view::size_type pos = view.find_first_of('='); pos != std::string_view::npos) {
                // there is an '=' in the argv, so we need to split, read find the argument and then parse its value
                option_iterator it = cliOptions.find(std::string(view.begin(), view.begin()+pos));

                // handle the "missing argument" case
                if (it == cliOptions.end()) {
                    // if we cannot ignore unknown args, we need to throw the NoSuchOptionException exception; otherwise, we simply skip it
                    if(!ignoreUnknownOptions) throw NoSuchOptionException(std::string(view.begin(), view.begin()+pos));
                    ++i;
                    continue;
                    
                }

                it->second->setArgFromInput(argv[i++]+pos+1);

            }
            else {
                
                option_iterator it = cliOptions.find(argv[i++]);

                if (it == cliOptions.end()) {
                    // if we cannot ignore unknown args, we need to throw the NoSuchOptionException exception; otherwise, we simply skip it
                    if(!ignoreUnknownOptions) throw NoSuchOptionException(argv[i-1]);
                    continue;
                }

                it->second->setArgFromInput(argv[i++]);  

            }
        }

        if (!suppressMissingRequiredOptionsError) {
            std::vector<std::string> missingReqOpt;
            for (option_iterator it = cliOptions.begin(); it != cliOptions.end(); ++it) {
                if (!it->second->good()) missingReqOpt.emplace_back(it->first); 
            }
            
            if (missingReqOpt.size() != 0) throw MissingRequiredOptionsError(missingReqOpt);
        }

    }

    std::vector<std::string> CliParser::getAllPossibleOptions() const {
        std::vector<std::string> res(cliOptions.size());
        // now cliOptions.size() == res.size()
        std::vector<std::string>::iterator resIt = res.begin(), resEnd;
        // we know that cliOptions.size() == res.size(), therefore, we can directly check optIt and ingore the check on resIt
        for (const_option_iterator optIt = cliOptions.begin(); optIt != cliOptions.end(); ++optIt) {
            *resIt++ = optIt->first;  // set *resIt to optIt->first and increment the resIt iterator to res
        }
        
        return res;
    }

    std::string CliParser::help(bool full, bool includeExecutablePath) const {
        // define some constant strings that will be used to form the help string
        std::string helpStr = appName;
        std::string optionHelpStr = "";
        for (const option_dictionary::value_type& item : cliOptions) {
            if (item.second->isOptional()) helpStr += " [" + item.first + "]";
            else helpStr += " " + item.first;

            if (full) optionHelpStr += item.first + "\t\t" + item.second->descr + "\n";
        }
        helpStr += "\n\n";
        // add the executablePath if it exists and includeExecutablePath is true
        if (includeExecutablePath && executablePath.size() != 0) helpStr += "installed at: " + executablePath + "\n\n";
        
        /*
        use std::move(optionHelpStr) when calling operator+
        Indeed, we do not have to create a "deep" copy of the string 
        since we return the result with a return statement
        and we will have no use of optionHelpStr after this operation 
        */
        return full ? helpStr + std::move(optionHelpStr) : helpStr;
    }

    template <> bool CliParser::parseArg<bool>(const char* input) {
        constexpr static const char* _yesStr = "y";
        constexpr static const char* _trueStr = "true";
        constexpr static const char* _noStr = "n";
        constexpr static const char* _falseStr = "false";
        constexpr static const std::string_view y(_yesStr, 1), t(_trueStr, 4), n(_noStr, 1), f(_falseStr, 5);
        static const std::string parsingError = "invalid bool argument";
        
        // convert the input to lowercase
        std::string in(input);
        for (std::string::iterator it = in.begin(); it != in.end(); ++it) {
            *it += (*it >= 'A' && *it <='Z')*32;
        }
        // now check the input
        if (y == in || t == in) return true;
        else if (n == in || f == in) return false;
        else throw std::invalid_argument(parsingError);
    }

}
