#ifndef FRIGOMESSAGE_H
#define FRIGOMESSAGE_H

#include <QObject>
#include <QJsonObject>
#include <QStringList>
#include <QList>
#include <QByteArray>
#include <QUuid>

class FrigoMessage : public QObject
{
    Q_OBJECT
public:
    explicit FrigoMessage(QObject *parent = 0);
    FrigoMessage(const QJsonObject &message, QObject *parent = 0);
    FrigoMessage(const QJsonObject &message, const QStringList targets, QObject *parent = 0);
    ~FrigoMessage();

    void to(QString target);
    void to(QStringList targets);
    void setSystem(bool system);

    QJsonObject getMessage() const;
    QStringList getTargets() const;
    QString getUuid() const;
    bool isSystem() const;
    qint64 getDelay() const;
    void setDelay(const qint64 &value);

    QJsonObject toJson();
    QByteArray serialize();
    QByteArray serializeBinary();
    static FrigoMessage *parse(const QByteArray &data, QObject *parent = 0);
    static FrigoMessage *parse(const QJsonObject &obj, QObject *parent = 0);
    static FrigoMessage *parseBinary(const QByteArray &data, QObject *parent = 0);

private:
    QUuid uuid;
    QJsonObject message;
    QStringList targets;
    bool system;
    qint64 delay;
};

typedef QList<FrigoMessage *> FrigoMessageList;

#endif // FRIGOMESSAGE_H
