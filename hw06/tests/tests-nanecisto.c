// #desc: Testy nanečisto
#include "cut.h"
#include "mainwrap.h"
#include "utils.h"

#include <stdlib.h>

// #desc: Vzorové vstupy
// #data: agents/sample.csv
// #data: worlds/village.csv
TEST(example)
{
    CHECK(app_main("--random-seed", "42", "agents/sample.csv", "worlds/village.csv") == 0);

    ASSERT_FILE(stdout, ("Random seed: 42\n"
                         "Virus is extinct.\n"
                         "Statistics:\n"
                         "	Total infections: 0\n"
                         "	Total deaths: 2\n"
                         "	Number of survivors: 3\n"
                         "Most infectious location:\n"
                         "	Multiple\n"
                         "Simulation terminated after 1 steps.\n"));
}

// #desc: Vzorové vstupy s argumentami navyše
// #data: agents/sample.csv
// #data: worlds/village.csv
TEST(example_with_args)
{
    CHECK(app_main("--random-seed", "1337", "--lethality", "0.8", "agents/sample.csv", "worlds/village.csv") == 0);

    ASSERT_FILE(stdout, ("Random seed: 1337\n"
                         "Virus is extinct.\n"
                         "Statistics:\n"
                         "	Total infections: 0\n"
                         "	Total deaths: 2\n"
                         "	Number of survivors: 3\n"
                         "Most infectious location:\n"
                         "	Multiple\n"
                         "Simulation terminated after 1 steps.\n"));
}

// #desc: Väčší vstup
// #data: agents/bigger.csv
// #data: worlds/bigger.csv
TEST(bigger_example)
{
    CHECK(app_main("--random-seed", "145", "agents/bigger.csv", "worlds/bigger.csv") == 0);

    ASSERT_FILE(stdout, ("Random seed: 145\n"
                         "Virus is extinct.\n"
                         "Statistics:\n"
                         "	Total infections: 14\n"
                         "	Total deaths: 43\n"
                         "	Number of survivors: 57\n"
                         "Most infectious location:\n"
                         "	Multiple\n"
                         "Simulation terminated after 4 steps.\n"));
}

// #desc: Neplatný argument na vstupe
// #data: agents/sample.csv
// #data: worlds/village.csv
TEST(invalid_arg)
{
    CHECK(app_main("--random-seed", "LOL", "agents/sample.csv", "worlds/village.csv") != 0);
}

// #desc: Chýbajúci argument na vstupe
TEST(missing_arg)
{
    CHECK(app_main(NULL) != 0);
}
