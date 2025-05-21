#ifndef DATABASE_H
#define DATABASE_H
#include <QSqlDatabase>
#include <QSqlError>
#include <QtConcurrent>

#define POSTGRE_DRIVER ("QPSQL")
#define DB_NAME ("MyDB")

enum fieldsForConnect
{
    hostName = 0,
    dbName = 1,
    userName = 2,
    password = 3,
    port = 4
};

class database : public QObject
{
    Q_OBJECT
public:
    database(QObject* parent = nullptr);
    void AddDataBase(QString driver, QString nameDB = "");
    void ConnectToDataBase(QVector<QString> data);
signals:
    void signalSendConnectionStatus(bool status);
public:
    QSqlDatabase* dataBase;
};

#endif // DATABASE_H
