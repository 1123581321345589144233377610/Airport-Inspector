#include "statisticwindow.h"
#include "mainwindow.h"
#include "ui_statisticwindow.h"

statisticwindow::statisticwindow(QWidget *parent) : QDialog{parent}, ui(new Ui::statisticwindow)
{
    ui->setupUi(this);
    connect(ui->closeWindow, &QPushButton::clicked, this, [&](){
        QDialog::close();
    });
    ui->months->addItem("январь");
    ui->months->addItem("февраль");
    ui->months->addItem("март");
    ui->months->addItem("апрель");
    ui->months->addItem("май");
    ui->months->addItem("июнь");
    ui->months->addItem("июль");
    ui->months->addItem("август");
    ui->months->addItem("сентябрь");
    ui->months->addItem("октябрь");
    ui->months->addItem("ноябрь");
    ui->months->addItem("декабрь");
    connect(this, &statisticwindow::SendAnnualLoadGraphic, this, &statisticwindow::DisplayAnnualLoadGraphic);
    connect(this, &statisticwindow::SendMonthlyLoadGraphic, this, &statisticwindow::DisplayMonthlyLoadGraphic);
    connect(ui->months, &QComboBox::currentTextChanged, this, &statisticwindow::MonthItemSelected);
}

void statisticwindow::SetAirportName(QString airportName)
{
    ui->airportNameLabel->setText(airportName);
}

statisticwindow::~statisticwindow()
{
    delete ui;
}

void statisticwindow::DisplayAnnualLoadGraphic(QChartView *chartView)
{
    for(int index = ui->annualLoadLayout->count() - 1; index >= 0; --index){
        QLayoutItem* item = ui->annualLoadLayout->itemAt(index);
        if(QWidget* widget = item->widget()){
            ui->annualLoadLayout->removeWidget(widget);
            delete widget;
        }
    }
    ui->annualLoadLayout->addWidget(chartView);
    chartView->show();
}

void statisticwindow::DisplayMonthlyLoadGraphic(QChartView *chartView)
{
    for(int index = ui->monthlyLoadLayout->count() - 1; index >= 0; --index){
        QLayoutItem* item = ui->monthlyLoadLayout->itemAt(index);
        if(QWidget* widget = item->widget()){
            ui->monthlyLoadLayout->removeWidget(widget);
            delete widget;
        }
    }
    ui->monthlyLoadLayout->addWidget(chartView);
    chartView->show();
}
