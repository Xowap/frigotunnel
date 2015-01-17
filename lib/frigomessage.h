#ifndef FRIGOMESSAGE_H
#define FRIGOMESSAGE_H

#include <QObject>
#include <QJsonObject>
#include <QStringList>

class FrigoMessage : public QObject
{
    Q_OBJECT
public:
    FrigoMessage(const QJsonObject &message, QObject *parent = 0);
    FrigoMessage(const QJsonObject &message, const QStringList targets, QObject *parent = 0);
    ~FrigoMessage();

    void to(QString target);
    void to(QStringList targets);

    QJsonObject getMessage() const;
    QStringList getTargets() const;

private:
    QJsonObject message;
    QStringList targets;
};

#endif // FRIGOMESSAGE_H
