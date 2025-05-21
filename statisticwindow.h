#ifndef STATISTICWINDOW_H
#define STATISTICWINDOW_H

#include <QWidget>
#include <QDialog>
#include <QChartView>

QT_BEGIN_NAMESPACE
namespace Ui{
class statisticwindow;
}
QT_END_NAMESPACE

class statisticwindow : public QDialog
{
    Q_OBJECT
public:
    explicit statisticwindow(QWidget *parent = nullptr);
    void SetAirportName(QString airportName);
    ~statisticwindow();
public slots:
    void DisplayAnnualLoadGraphic(QChartView* chartView);
    void DisplayMonthlyLoadGraphic(QChartView* chartView);
signals:
    void SendAnnualLoadGraphic(QChartView* chartView);
    void SendMonthlyLoadGraphic(QChartView* chartView);
    void MonthItemSelected(QString month);
private:
    Ui::statisticwindow* ui;
};

#endif // STATISTICWINDOW_H
