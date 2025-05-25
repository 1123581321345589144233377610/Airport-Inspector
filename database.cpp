#include "database.h"

database::database(QObject* parent) : QObject(parent)
{
    dataBase = new QSqlDatabase();
}

void database::AddDataBase(QString driver, QString nameDB)
{
    *dataBase = QSqlDatabase::addDatabase(driver, nameDB);
}

void database::ConnectToDataBase(QVector<QString> data)
{
    if(dataBase->isOpen()){
        dataBase->close();
    }
    QThread* thread = new QThread();
    connect(thread, &QThread::started, [=](){
        dataBase->setHostName(data[hostName]);
        dataBase->setDatabaseName(data[dbName]);
        dataBase->setUserName(data[userName]);
        dataBase->setPassword(data[password]);
        dataBase->setPort(data[port].toInt());
        bool status = dataBase->open();
        if(!status){
            qDebug() << "Ошибка подключения: " << dataBase->lastError().text();
        }
        emit signalSendConnectionStatus(status);
        thread->quit();
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

database::~database()
{
    delete dataBase;
}
