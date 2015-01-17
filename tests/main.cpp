#include <QCoreApplication>
#include <algorithm>

#include "parsepackettest.h"
#include "expiringsettest.h"

int main(int argc, char *argv[])
{
    int output = 0;
    QCoreApplication a(argc, argv);

    ParsePacketTest ppt;
    output += QTest::qExec(&ppt, argc, argv);

    ExpiringSetTest est;
    output += QTest::qExec(&est, argc, argv);

    return std::min(output, 1);
}
