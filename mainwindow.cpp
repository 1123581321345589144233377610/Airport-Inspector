#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dataBaseStatus->setText("ОТКЛЮЧЕНО");
    ui->comboBox->setEnabled(false);
    ui->dateEdit->setEnabled(false);
    ui->radioButtonArrival->setEnabled(false);
    ui->radioButtonDeparture->setEnabled(false);
    ui->getFlightSchedule->setEnabled(false);
    ui->getLoadStatistic->setEnabled(false);
    ui->radioButtonArrival->setChecked(true);
    dataBase = new database(this);
    dataBase->AddDataBase(POSTGRE_DRIVER, DB_NAME);
    query = new QSqlQuery();
    dataToConnect = {"981757-ca08998.tmweb.ru", "demo", "netology_usr_cpp", "CppNeto3", "5432"};
    reconnectTimer = new QTimer(this);
    connect(reconnectTimer, &QTimer::timeout, this, [&](){
        dataBase->ConnectToDataBase(dataToConnect);
    });
    connect(dataBase, &database::signalSendConnectionStatus, this, &MainWindow::ReceiveStatusConnectionToDataBase);
    dataBase->ConnectToDataBase(dataToConnect);
}

void MainWindow::ReceiveListOfAirports()
{
    *query = QSqlQuery(*dataBase->dataBase);
    bool statusRequest = query->exec(QString("SELECT airport_name->>'ru' as %1, airport_code FROM bookings.airports_data").arg("airportName"));
    if(!statusRequest){
        if(!messageBoxForError){
            messageBoxForError = new QMessageBox(this);
            messageBoxForError->setWindowTitle("Request error!");
            messageBoxForError->setIcon(QMessageBox::Critical);
        }
        messageBoxForError->setText(query->lastError().text());
        messageBoxForError->show();
    }else{
        while(query->next()){
            ui->comboBox->addItem(query->value(0).toString(), query->value(1).toString());
        }
    }
}

void MainWindow::GetFlightShedule()
{
    ui->tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QString airportCode = ui->comboBox->currentData().toString();
    QDate selectedDate = ui->dateEdit->date();
    QString direction = (ui->radioButtonDeparture->isChecked()) ? "departure" : "arrival";
    QString queryTemplate = (direction == "departure")
        ? QString("SELECT flight_no, scheduled_departure::time, "
            "ad.airport_name->>'ru' AS \"Name\" "
            "FROM bookings.flights f "
            "JOIN bookings.airports_data ad ON ad.airport_code = f.arrival_airport "
            "WHERE f.departure_airport = :airportCode "
            "AND f.scheduled_departure::date = :selectedDate "
            "ORDER BY f.scheduled_departure")
        : QString( "SELECT flight_no, scheduled_arrival::time, "
            "ad.airport_name->>'ru' AS \"Name\" "
            "FROM bookings.flights f "
            "JOIN bookings.airports_data ad ON ad.airport_code = f.departure_airport "
            "WHERE f.arrival_airport = :airportCode "
            "AND f.scheduled_arrival::date = :selectedDate "
            "ORDER BY f.scheduled_arrival");
    query->prepare(queryTemplate);
    query->bindValue(":airportCode", airportCode);
    query->bindValue(":selectedDate",selectedDate.toString(Qt::ISODate));
    bool statusRequest = query->exec();
    if(!statusRequest){
        if(!messageBoxForError){
            messageBoxForError = new QMessageBox(this);
            messageBoxForError->setWindowTitle("Request error!");
            messageBoxForError->setIcon(QMessageBox::Critical);
        }
        messageBoxForError->setText(query->lastError().text());
        messageBoxForError->show();
    }else{
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(3);
        QStringList headers;
        if(direction == "departure"){
            headers = {"Номер рейса", "Время вылета", "Аэропорт назначения"};
        }else{
            headers = {"Номер рейса", "Время вылета", "Аэропорт отправления"};
        }
        ui->tableWidget->setHorizontalHeaderLabels(headers);
        uint32_t rowCounter = 0;
        while(query->next()){
            ui->tableWidget->insertRow(rowCounter);
            QString flightNumber = query->value(0).toString();
            QString flightTime = query->value(1).toString();
            QString airportName = query->value(2).toString();
            QTableWidgetItem* flightItem = new QTableWidgetItem(flightNumber);
            QTableWidgetItem* timeItem = new QTableWidgetItem(flightTime);
            QTableWidgetItem* airportItem = new QTableWidgetItem(airportName);
            flightItem->setTextAlignment(Qt::AlignCenter);
            timeItem->setTextAlignment(Qt::AlignCenter);
            airportItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(rowCounter, 0, flightItem);
            ui->tableWidget->setItem(rowCounter, 1, timeItem);
            ui->tableWidget->setItem(rowCounter, 2, airportItem);
            ui->tableWidget->resizeColumnsToContents();
            ++rowCounter;
        }
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
}

void MainWindow::GetAnnualLoadGraphic()
{
    QString airportCode = ui->comboBox->currentData().toString();
    query->prepare("SELECT count(flight_no), date_trunc('month', scheduled_departure) as \"Month\" from bookings.flights f"
        " WHERE (scheduled_departure::date > date('2016-08-31') and scheduled_departure::date <= date('2017-08-31'))"
        " and ( departure_airport = :airportCode or arrival_airport = :airportCode )"
        " group by \"Month\"");
    query->bindValue(":airportCode", airportCode);
    bool statusRequest = query->exec();
    if(!statusRequest){
        if(!messageBoxForError){
            messageBoxForError = new QMessageBox(this);
            messageBoxForError->setWindowTitle("Request error! Cannot recieve annual load!");
            messageBoxForError->setIcon(QMessageBox::Critical);
        }
        messageBoxForError->setText(query->lastError().text());
        messageBoxForError->show();
    }else{
        QBarSeries* barGraph = new QBarSeries();
        QBarSet* set = new QBarSet("КОЛИЧЕСТВО ВЫЛЕТОВ/ПРИЛЁТОВ");
        QStringList months;
        while(query->next()){
            QDateTime monthDateTime = query->value("Month").toDateTime();
            QDate monthDate = monthDateTime.date();
            QString monthName = QLocale().monthName(monthDate.month(), QLocale::ShortFormat);
            int flightCount = query->value("count").toInt();
            set->append(flightCount);
            months.append(monthName);
        }
        barGraph->append(set);
        QChart* chart = new QChart();
        chart->addSeries(barGraph);
        chart->setAnimationOptions(QChart::SeriesAnimations);
        QBarCategoryAxis* axisX = new QBarCategoryAxis();
        axisX->append(months);
        chart->addAxis(axisX, Qt::AlignBottom);
        barGraph->attachAxis(axisX);
        QValueAxis* axisY = new QValueAxis();
        chart->addAxis(axisY, Qt::AlignLeft);
        barGraph->attachAxis(axisY);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        QChartView* chartView = new QChartView(chart, dialog);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setVisible(true);
        emit dialog->SendAnnualLoadGraphic(chartView);
    }
}

int MainWindow::GetMonthNumberFromRussianName(const QString& monthName)
{
    QMap<QString, int> monthMap = {
        {"январь", 1}, {"февраль", 2}, {"март", 3}, {"апрель", 4},
        {"май", 5}, {"июнь", 6}, {"июль", 7}, {"август", 8},
        {"сентябрь", 9}, {"октябрь", 10}, {"ноябрь", 11}, {"декабрь", 12}
    };
    QString normalized = monthName.toLower().trimmed();
    if (!monthMap.contains(normalized)) {
        return -1;
    }
    return monthMap[normalized];
}

void MainWindow::GetMonthlyLoad(QString month = "Январь")
{
    QString airportCode = ui->comboBox->currentData().toString();
    int monthNumber = GetMonthNumberFromRussianName(month);
    if(monthNumber == -1){
        if(!messageBoxForError){
            messageBoxForError = new QMessageBox(this);
            messageBoxForError->setIcon(QMessageBox::Critical);
        }
        messageBoxForError->setWindowTitle("Undefined month!");
        messageBoxForError->show();
        return;
    }
    int year = (monthNumber >= 9) ? 2016 : 2017;
    QDate monthStart = QDate(year, monthNumber, 1);
    QDate monthEnd = monthStart.addMonths(1);
    int firstDay = 1;
    int lastDay = monthStart.daysInMonth();
    query->prepare("SELECT count(flight_no), date_trunc('day', scheduled_departure) as \"Day\""
        " FROM bookings.flights f"
        " WHERE scheduled_departure::date >= date(:monthStart)"
        " AND scheduled_departure::date < date(:monthEnd)"
        " AND (departure_airport = :airportCode OR arrival_airport = :airportCode)"
        " GROUP BY \"Day\"");
    query->bindValue(":monthStart", monthStart.toString(Qt::ISODate));
    query->bindValue(":monthEnd", monthEnd.toString(Qt::ISODate));
    query->bindValue(":airportCode", airportCode);
    bool statusRequest = query->exec();
    if(!statusRequest){
        if(!messageBoxForError){
            messageBoxForError = new QMessageBox(this);
            messageBoxForError->setWindowTitle("Request error! Cannot recieve monthly load!");
            messageBoxForError->setIcon(QMessageBox::Critical);
        }
        messageBoxForError->setText(query->lastError().text());
        messageBoxForError->show();
    }else{
        QLineSeries* lineGraph = new QLineSeries();
        lineGraph->setName("КОЛИЧЕСТВО ВЫЛЕТОВ/ПРИЛЁТОВ");
        while(query->next()){
            QDateTime dayDateTime = query->value("Day").toDateTime();
            QDate date = dayDateTime.date();
            int dayNumber = date.day();
            int flightCount = query->value("count").toInt();
            lineGraph->append(dayNumber, flightCount);
        }
        QChart* chart = new QChart();
        chart->addSeries(lineGraph);
        chart->setAnimationOptions(QChart::SeriesAnimations);
        QValueAxis* axisX = new QValueAxis();
        axisX->setRange(firstDay, lastDay);
        axisX->setLabelFormat("%.0f");
        axisX->setTickInterval(1);
        axisX->setTickCount(lastDay + 1);
        chart->addAxis(axisX, Qt::AlignBottom);
        lineGraph->attachAxis(axisX);
        QValueAxis* axisY = new QValueAxis();
        chart->addAxis(axisY, Qt::AlignLeft);
        lineGraph->attachAxis(axisY);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        QChartView* chartView = new QChartView(chart, dialog);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setVisible(true);
        emit dialog->SendMonthlyLoadGraphic(chartView);
    }
}

void MainWindow::ReceiveNewMonth(QString month)
{
    GetMonthlyLoad(month);
}

void MainWindow::GetLoadStatistic()
{
    if(!dialog){
        dialog = new statisticwindow(this);
        connect(dialog, &statisticwindow::MonthItemSelected, this, &MainWindow::ReceiveNewMonth);
    }
    dialog->SetAirportName(ui->comboBox->currentText());
    GetAnnualLoadGraphic();
    GetMonthlyLoad();
    dialog->exec();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete dialog;
    delete query;
}

void MainWindow::ReceiveStatusConnectionToDataBase(bool status)
{
    if(status){
        ui->dataBaseStatus->setText("ПОДКЛЮЧЕНО");
        ui->comboBox->setEnabled(true);
        ui->dateEdit->setEnabled(true);
        ui->radioButtonArrival->setEnabled(true);
        ui->radioButtonDeparture->setEnabled(true);
        ui->getFlightSchedule->setEnabled(true);
        ui->getLoadStatistic->setEnabled(true);
        QDate minDate(2016, 8, 15);
        QDate maxDate(2017, 9, 14);
        ui->dateEdit->setMinimumDate(minDate);
        ui->dateEdit->setMaximumDate(maxDate);
        ui->dateEdit->setDate(minDate);
        ReceiveListOfAirports();
        connect(ui->getFlightSchedule, &QPushButton::clicked, this, &MainWindow::GetFlightShedule);
        connect(ui->getLoadStatistic, &QPushButton::clicked, this, &MainWindow::GetLoadStatistic);
    }else{
        if(!messageBoxForError){
            messageBoxForError = new QMessageBox(this);
            messageBoxForError->setWindowTitle("Database connection error!");
            messageBoxForError->setIcon(QMessageBox::Critical);
            messageBoxForError->setDefaultButton(QMessageBox::Ok);
            connect(messageBoxForError, &QMessageBox::buttonClicked, this, [&](){
                reconnectTimer->start(5000);
            });
        }
        messageBoxForError->setText(dataBase->dataBase->lastError().text());
        messageBoxForError->show();
    }
}
