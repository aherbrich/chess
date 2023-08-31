#!/bin/sh
set -e

# try to make all targets without sanitizers
make clean && make all

# run tests without leak check
echo "Running tests without leak check..."
echo "\n\033[0;35mm====================================== [test_factor_graph] ======================================\033[0m\n"
./build/tests/test_factor_graph $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
echo "\n\033[0;35mm====================================== [test_eval] ======================================\033[0m\n"
./build/tests/test_eval $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
echo "\n\033[0;35mm====================================== [test_perftdivide] ======================================\033[0m\n"
./build/tests/test_perftdivide $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
echo "\n\033[0;35mm====================================== [test_tt] ======================================\033[0m\n"
./build/tests/test_tt $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
# echo "\n\033[0;35mm====================================== [test_movegen] ======================================\033[0m\n"
# ./build/tests/test_movegen $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";

# run tests with leak check
echo "Running tests with leak check..."
echo "\n\033[0;35mm====================================== [test_factor_graph + leak check] ======================================\033[0m\n"
sudo leaks -atExit -- ./build/tests/test_factor_graph
echo "\n\033[0;35mm====================================== [test_eval + leak check] ======================================\033[0m\n"
sudo leaks -atExit -- ./build/tests/test_eval 
echo "\n\033[0;35mm====================================== [test_perftdivide + leak check] ======================================\033[0m\n"
sudo leaks -atExit -- ./build/tests/test_perftdivide
echo "\n\033[0;35mm====================================== [test_tt + leak check] ======================================\033[0m\n"
sudo leaks -atExit -- ./build/tests/test_tt
echo "\n\033[0;35mm====================================== [test_movegen + leak check] ======================================\033[0m\n"
sudo leaks -atExit -- ./build/tests/test_movegen

# try to compile all targets with -Werror and run tests with sanitizers activated
make clean && make sanitize

# run the tests
echo "Running tests..."
echo "\n\033[0;35mm====================================== [test_factor_graph + sanitizers] ======================================\033[0m\n"
./build/tests/test_factor_graph $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
echo "\n\033[0;35mm====================================== [test_eval + sanitizers] ======================================\033[0m\n"
./build/tests/test_eval $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
echo "\n\033[0;35mm====================================== [test_perftdivide + sanitizers] ======================================\033[0m\n"
./build/tests/test_perftdivide $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
echo "\n\033[0;35mm====================================== [test_tt + sanitizers] ======================================\033[0m\n"
./build/tests/test_tt $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
# echo "\n\033[0;35mm====================================== [test_movegen + sanitizers] ======================================\033[0m\n"
# ./build/tests/test_movegen $? -eq 0 && echo "\n<<< \033[0;32mOK\033[0m" || echo "\n<<< \033[0;31mFAIL\033[0m";
