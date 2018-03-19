#include "server.h"
#include "logic.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server w;
    Logic logic;

    QObject::connect(&w, SIGNAL(dataReceived(int, QString)), &logic, SLOT(processData(int, QString)));
    QObject::connect(&w, SIGNAL(dataReceived(int, int)), &logic, SLOT(processData(int, int)));
    QObject::connect(&w, SIGNAL(NewPlayer(QTcpSocket*)), &logic, SLOT(EasyNumber(QTcpSocket*)));
    QObject::connect(&logic, SIGNAL(YourNumber(QTcpSocket*, int)), &w, SLOT(AddCharacter(QTcpSocket*, int)));
    QObject::connect(&logic, SIGNAL(RoleByNumber(int, int)), &w, SLOT(writeData(int,int)));
    QObject::connect(&logic, SIGNAL(JustSpeech(QString)), &w, SLOT(writeData(QString)));
    QObject::connect(&logic, SIGNAL(MafiaOrders(vector<int>, QString)), &w, SLOT(MafiaOrders(vector<int>, QString)));
    QObject::connect(&logic, SIGNAL(CheckResult(int, int)), &w, SLOT(writeData(int, int)));
    QObject::connect(&logic, SIGNAL(DeadPlayer(int)), &w, SLOT(writeData(int)));
    QObject::connect(&logic, SIGNAL(HereIsTheWinner(QString)), &w, SLOT(writeData(QString)));
    QObject::connect(&logic, SIGNAL(PlayerSpeaking(int)), &w, SLOT(writeData(int)));


    return a.exec();
}
