#include <gtest/gtest.h>
#include <ews/ews_test_support.hpp>

#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
# include <direct.h>
# define getcwd _getcwd
#else
# include <unistd.h>
#endif

typedef std::unordered_map<std::string, std::string> argument_map;

namespace
{
    bool starts_with(const char* prefix, const std::string& str)
    {
        return !str.compare(0, std::strlen(prefix), prefix);
    }

    bool is(const char* prefix, const std::string& arg, argument_map& map)
    {
        if (starts_with(prefix, arg))
        {
            const auto value = arg.substr(std::strlen(prefix));
            map[prefix] = value;
            return true;
        }
        return false;
    }

    std::string pwd()
    {
        enum { size = 4096U };
        char buf[size];
        auto ret = ::getcwd(buf, size);
        return ret != nullptr ?
                      std::string(buf)
                    : throw std::runtime_error("getcwd failed");
    }

    // Parse and interpret command line flags that we recognize. Remove them
    // from argv. Leave the rest untouched.
    void init_from_args(int* argc, char** argv)
    {
        argument_map args;

        for (int i = 1; i < *argc; i++)
        {
            const auto arg = std::string(argv[i]);

            // Do we see a flag that we recognize?
            if (is("--assets=", arg, args))
            {
                // Finally, shift remainder of argv left by one. Note that argv
                // has (*argc + 1) elements, the last one always being null.
                // The following loop moves the trailing null element as well.
                for (int j = i; j != *argc; j++)
                {
                  argv[j] = argv[j + 1];
                }

                // Decrements the argument count.
                (*argc)--;

                // We also need to decrement the iterator as we just removed an
                // element.
                i--;
            }
            else if (  arg == "--help"
                    || arg == "-h"
                    || arg == "-?"
                    || arg == "/?")
            {
                std::cout <<
"usage: tests [--assets=PATH]\n"
"\n"
"--assets=PATH  path to the test assets directory, default\n"
"               $PWD/tests/assets\n"
"\n"
"invoke with --gtest_help to see Google Test flags" << std::endl;

                std::exit(EXIT_FAILURE);
            }
            else if (is("--gtest_help", arg, args))
            {
                return;
            }
        }

        auto assets_dir = std::string(pwd() + "/tests/assets");
        auto it = args.find("--assets=");
        if (it != args.end())
        {
            assets_dir = it->second;
        }
        ews::test::global_data::instance().assets_dir = assets_dir;
        std::cout << "Loading assets from: '" << assets_dir << "'\n";
    }
}

int main(int argc, char** argv)
{
    init_from_args(&argc, argv);
    std::cout << "Running tests from main.cpp\n";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// vim:et ts=4 sw=4 noic cc=80
