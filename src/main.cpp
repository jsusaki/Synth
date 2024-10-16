#include "Core/Application.h"

int main()
{
    Application app;
    if (app.Init()) app.Start();
    app.ShutDown();
    return 0;
}