#ifndef SERVER_H
#define SERVER_H

#include <QtCore>
#include <QtNetwork>
#include <map>
#include <vector>

using namespace std;

namespace Ui {
class Server;
}

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = 0);
signals:
    void dataReceived(int, QString);
    void dataReceived(int, int);
    void NewPlayer(QTcpSocket*);
public slots:
    void AddCharacter(QTcpSocket*, int);
    void newConnection();
    void disconnected();
    void readyRead();
    bool writeData(int, QString);
    bool writeData(int, int);
    int readInt(QByteArray*, qint32);
    QString readString(QByteArray*, qint32);
    bool writeData(QString);
    bool writeData(int);
    void MafiaOrders(vector<int>, QString);

private:
    map<QTcpSocket*, int> characters;
    QTcpServer *server;
    QHash<QTcpSocket*, QByteArray*> buffers; //We need a buffer to store data until block has completely received
    QHash<QTcpSocket*, qint32*> sizes; //We need to store the size to verify if a block has received completely
};

#endif // SERVER_H
