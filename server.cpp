#include "server.h"
#include <iostream>

static inline qint32 ArrayToInt(QByteArray source);
static inline QByteArray IntToArray(qint32 source);

Server::Server(QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), SLOT(newConnection()));
    qDebug() << "Listening:" << server->listen(QHostAddress::Any, 1024);
}

void Server::newConnection()
{
    while (server->hasPendingConnections())
    {
        qDebug()<<"New connection\n";
        QTcpSocket *socket = server->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
        QByteArray *buffer = new QByteArray();
        qint32 *s = new qint32(0);
        buffers.insert(socket, buffer);
        sizes.insert(socket, s);
        emit NewPlayer(socket);
    }
}

void Server::disconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);
    qint32 *s = sizes.value(socket);
    socket->deleteLater();
    delete buffer;
    delete s;
}

void Server::readyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);
    qint32 *s = sizes.value(socket);
    qint32 size = *s;
    while (socket->bytesAvailable() > 0)
    {
        buffer->append(socket->readAll());
        while ((size == 0 && buffer->size() >= 4) || (size > 0 && buffer->size() >= size)) //While can process data, process it
        {
            if (size == 0 && buffer->size() >= 4) //if size of data has received completely, then store it on our global variable
            {
                size = ArrayToInt(buffer->mid(0, 4));
                *s = size;
                buffer->remove(0, 4);
            }
            if (size > 0 && buffer->size() >= size) // If data has received completely, then emit our SIGNAL with the data
            {
                QByteArray coding = buffer->mid(0, size);
                buffer->remove(0, size);
                size = 0;
                *s = size;
                qDebug()<<"Code recieved";
                int code = ArrayToInt(coding);
                qDebug()<<QString::number(code);
                switch(code)
                {
                case 0:
                {
                    emit dataReceived(characters[socket], readInt(buffer, size));
                    break;
                }
                case 1:
                {
                    emit dataReceived(characters[socket], readString(buffer, size));
                    break;
                }
                }
            }
        }
    }
}

bool Server::writeData(int num, QString data)
{
    QTcpSocket* socket;
    auto findResult = find_if(begin(characters), end(characters), [&](const pair<QTcpSocket*, int> &pair)
    {
        return pair.second == num;
    });
    if (findResult != end(characters))
        socket = findResult->first;
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray code = IntToArray(1);
        socket->write(IntToArray(code.size()));
        socket->write(code);
        socket->write(IntToArray(data.toUtf8().size()));
        socket->write(data.toUtf8());
        return socket->waitForBytesWritten();
    }
    else
        return false;
}
bool Server::writeData(int num, int data)
{
    QTcpSocket* socket;
    auto findResult = find_if(begin(characters), end(characters), [&](const pair<QTcpSocket*, int> &pair)
    {
        return pair.second == num;
    });
    if (findResult != end(characters))
        socket = findResult->first;
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug()<<"I'm writing " + QString::number(data) + " to guy #" + QString::number(num) + "!\n";
        QByteArray temp = IntToArray(data);
        QByteArray code = IntToArray(0);
        socket->write(IntToArray(code.size()));
        socket->write(code);
        socket->write(IntToArray(temp.size()));
        socket->write(temp);
        return socket->waitForBytesWritten();
    }
    else
        return false;
}

bool Server::writeData(int data)
{
    for(const auto& pair : characters)
    {
        QTcpSocket* socket = pair.first;
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        qDebug()<<"I'm writing " + QString::number(data) + " to everyone!\n";
        QByteArray temp = IntToArray(data);
        QByteArray code = IntToArray(0);
        socket->write(IntToArray(code.size()));
        socket->write(code);
        socket->write(IntToArray(temp.size()));
        socket->write(temp);
        return socket->waitForBytesWritten();
    }
    else
        return false;
    }
}
bool Server::writeData(QString data)
{
    for(const auto& pair : characters)
    {
        QTcpSocket* socket = pair.first;
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray code = IntToArray(1);
        socket->write(IntToArray(code.size()));
        socket->write(code);
        socket->write(IntToArray(data.toUtf8().size()));
        socket->write(data.toUtf8());
        qDebug()<<"Data written: " + data + "\n";
        return socket->waitForBytesWritten();
    }
    else
        return false;
    }
}

int Server::readInt(QByteArray* buffer, qint32 size)
{
    if (size == 0 && buffer->size() >= 4) //if size of data has received completely, then store it on our global variable
    {
        size = ArrayToInt(buffer->mid(0, 4));
        buffer->remove(0, 4);
    }
    if (size > 0 && buffer->size() >= size) // If data has received completely, then emit our SIGNAL with the data
    {
        QByteArray data = buffer->mid(0, size);
        buffer->remove(0, size);
        size = 0;
        qDebug()<<"Data recieved";
        qDebug()<<QString::number(ArrayToInt(data));
        return ArrayToInt(data);
    }
}
QString Server::readString(QByteArray* buffer, qint32 size)
{
    if (size == 0 && buffer->size() >= 4) //if size of data has received completely, then store it on our global variable
    {
        size = ArrayToInt(buffer->mid(0, 4));
        buffer->remove(0, 4);
    }
    if (size > 0 && buffer->size() >= size) // If data has received completely, then emit our SIGNAL with the data
    {
        QByteArray data = buffer->mid(0, size);
        buffer->remove(0, size);
        size = 0;
        qDebug()<<"Data recieved";
        qDebug()<<data;
        QString text(data);
        return text;
    }
}

void Server::MafiaOrders(vector<int> mafias, QString orders)
{
    for (int i = 0; i < mafias.size(); i++)
    {
        writeData(mafias[i], orders);
    }
}

void Server::AddCharacter(QTcpSocket* socket, int num)
{
    characters[socket] = num;
    writeData(num, num);
}

qint32 ArrayToInt(QByteArray source)
{
    qint32 temp;
    QDataStream data(&source, QIODevice::ReadWrite);
    data >> temp;
    return temp;
}

QByteArray IntToArray(qint32 source) //Use qint32 to ensure that the number have 4 bytes
{
    //Avoid use of cast, this is the Qt way to serialize objects
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}


