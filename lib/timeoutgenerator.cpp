#include "timeoutgenerator.h"

#include <chrono>

TimeoutGenerator::TimeoutGenerator(int min, int max, QObject *parent) :
    QObject(parent),
    generator(std::chrono::system_clock::now().time_since_epoch().count()),
    distribution(std::uniform_int_distribution<int>(min, max))
{
}

TimeoutGenerator::~TimeoutGenerator()
{
}

int TimeoutGenerator::generate()
{
    return distribution(generator);
}

