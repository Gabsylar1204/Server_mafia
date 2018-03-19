#ifndef LOGIC_H
#define LOGIC_H

#include <QObject>
#include "player.h"
#include <vector>
#include <map>
#include <QtCore>
#include <QtNetwork>

using namespace std;

enum STATE {FIRST_NIGHT, DEAD_PLAYER, DISCUSSION, VOTING, SHOOTING, CHECKING};

//ДОПИСАТЬ СВИТЧСТЕЙТ ДЛЯ ЧЕКИНГ!!!

class Logic : public QObject
{
    Q_OBJECT
public:
    explicit Logic(QObject *parent = nullptr);
    int killedByVotes();
    int killedByShots();
    QString parsedForVote(int, QString);
    bool ShotsEqual();
    int ItsCountingTime();
    void switchState();
    void checkForWinner();
signals:
    void YourNumber(QTcpSocket*, int);
    void RoleByNumber(int, int);
    void JustSpeech(QString);
    void MafiaOrders(vector<int>, QString);
    void CheckResult(int, int);
    void DeadPlayer(int);
    void HereIsTheWinner(QString);
    void PlayerSpeaking(int);
public slots:
    void processData(int, int);
    void processData(int, QString);
    void EasyNumber(QTcpSocket*);
private:
    STATE s = FIRST_NIGHT;
    int checks = 0;
    int dead_player = 0;
    bool daytime = false;
    vector<Player> players;
    map<int,int> voted;
    vector<int> shots;
    vector<int> roles = {0, 0, 0, 0, 0, 0, 1, -2, -2, -2};
    vector<int> numbers = {1};//, 2, 3, 4, 5, 6, 7, 8, 9, 10};
};

#endif // LOGIC_H
