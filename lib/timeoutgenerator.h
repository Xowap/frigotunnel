#ifndef TIMEOUTGENERATOR_H
#define TIMEOUTGENERATOR_H

#include <QObject>
#include <random>

class TimeoutGenerator : public QObject
{
    Q_OBJECT
public:
    explicit TimeoutGenerator(int min, int max, QObject *parent = 0);
    ~TimeoutGenerator();

    int generate();

private:
    int min, max;

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution;
};

#endif // TIMEOUTGENERATOR_H
