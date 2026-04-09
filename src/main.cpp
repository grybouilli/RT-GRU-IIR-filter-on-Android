#include <App.hpp>

std::atomic<bool> run = true;
void sigint_handler(int arg)
{
    run = false;
}

int main(int argc, char ** argv)
{
    signal(SIGINT, sigint_handler);

    App app(argc, argv, run);
    app.run();
    return  EXIT_SUCCESS;
}