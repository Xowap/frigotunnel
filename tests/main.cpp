#include <QCoreApplication>
#include <algorithm>

#include "parsepackettest.h"
#include "expiringsettest.h"
#include "frigotunneltest.h"
#include "frigoconnectiontest.h"
#include "timeoutgeneratortest.h"

int main(int argc, char *argv[])
{
    int output = 0;
    QCoreApplication a(argc, argv);

    ParsePacketTest ppt;
    output += QTest::qExec(&ppt, argc, argv);

    ExpiringSetTest est;
    output += QTest::qExec(&est, argc, argv);

    FrigoTunnelTest ftt;
    output += QTest::qExec(&ftt, argc, argv);

    FrigoConnectionTest fct;
    output += QTest::qExec(&fct, argc, argv);

    TimeoutGeneratorTest tgt;
    output += QTest::qExec(&tgt, argc, argv);

    return std::min(output, 1);
}
