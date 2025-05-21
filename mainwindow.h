#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QSqlQuery>
#include <QtCharts>
#include <QChartView>
#include "database.h"
#include "statisticwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void ReceiveListOfAirports();
    void GetFlightShedule();
    void GetLoadStatistic();
    void GetAnnualLoadGraphic();
    void GetMonthlyLoad(QString month);
    int GetMonthNumberFromRussianName(const QString& monthName);
    ~MainWindow();
public slots:
    void ReceiveStatusConnectionToDataBase(bool status);
    void ReceiveNewMonth(QString month);
private:
    Ui::MainWindow *ui;
    database* dataBase;
    QMessageBox* messageBoxForError = nullptr;
    QVector<QString> dataToConnect;
    QTimer* reconnectTimer;
    QSqlQuery* query;
    statisticwindow* dialog = nullptr;
};
#endif // MAINWINDOW_H
