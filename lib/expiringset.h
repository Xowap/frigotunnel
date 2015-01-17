#ifndef EXPIRINGSET_H
#define EXPIRINGSET_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>
#include <QTimer>

typedef QVector<QSet<QString> > ExpiringSetBuffer;
class ExpiringSetTest;

class ExpiringSet : public QObject
{
    Q_OBJECT

    friend class ExpiringSetTest;

public:
    ExpiringSet(int cycles, QObject *parent = 0);
    ExpiringSet(int cycles, int period, QObject *parent = 0);
    ~ExpiringSet();

    void add(QString key);
    bool contains(QString key);

public slots:
    void cycle();

private:
    int cycles;
    int currentCycle;
    int period;
    ExpiringSetBuffer buffer;
    QTimer cycleTimer;

    int cycleAfter(int cycle);
};

#endif // EXPIRINGSET_H
