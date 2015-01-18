#ifndef TIMEOUTGENERATORTEST_H
#define TIMEOUTGENERATORTEST_H

#include <QObject>

class TimeoutGeneratorTest : public QObject
{
    Q_OBJECT
public:
    explicit TimeoutGeneratorTest(QObject *parent = 0);
    ~TimeoutGeneratorTest();

private slots:
    void generateRandom();
};

#endif // TIMEOUTGENERATORTEST_H
