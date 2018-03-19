#include "logic.h"
#include <algorithm>

using namespace std;

Logic::Logic(QObject *parent) : QObject(parent)
{
    Player pl;
    for (int i = 0; i < 10; i++)
        players.push_back(pl);
    for (int i = 0; i < players.size(); i++)
        players[i].number = i+1;
    random_shuffle(roles.begin(), roles.end());
    random_shuffle(numbers.begin(), numbers.end());
}

void Logic::processData(int num, int data)
{
    switch(s)
    {
    case FIRST_NIGHT:
        players[num - 1].role = roles[data - 1];
        emit RoleByNumber(num, roles[data - 1]);
        break;
    case CHECKING:
        for(int i = 0; i < players.size(); i++)
        {
            if(players[i].number == data)
            {
                emit CheckResult(num, players[i].role);
                checks++;
            }
        }
        if(checks == 1)
        {
            emit JustSpeech("Switch.\n");
            switchState();
            checks = 0;
        }
        break;
    case SHOOTING:
        shots.push_back(data);
        break;
    case VOTING:
        voted[data] = voted[data] + 1;
        break;
    }
}
void Logic::processData(int num, QString speech)
{
    switch(s)
    {
    case FIRST_NIGHT:
    {
        if(speech == "Switch.\n")
        {
            switchState();
            break;
        }
        vector<int> mafias;
        for (int i = 0; i < players.size(); i++)
        {
            if (players[i].role < 0)
                mafias.push_back(players[i].number);
        }
        emit MafiaOrders(mafias, speech);
        break;
    }
    case DISCUSSION:
    {
        QString new_speech = parsedForVote(num, speech);
        emit JustSpeech(new_speech);
        for (int i = 0; i < players.size(); i++)
        {
            if (players[i].number == num)
                players[i].my_speech = true;
        }
        int speaking = 0;
        for (int i = num - 1; i < players.size(); i++)
            if (players[i].is_dead == false && players[i].my_speech == false && players[i].role != 0)
            {
                speaking = players[i].number;
                emit PlayerSpeaking(speaking);
                break;
            }
        if (speaking == 0)
        {
            switchState();
            for (auto player : players)
                player.my_speech = false;
            emit JustSpeech("Switch.\n");
        }
        break;
    }
    case DEAD_PLAYER:
        emit JustSpeech(parsedForVote(num, speech));
        switchState();
        break;
    default:
        emit DeadPlayer(ItsCountingTime());
        break;
    }
}
int Logic::killedByVotes()
{
    vector<int> vote_count;
    for (const auto& pair : voted )
    {
        int j = pair.second;
        vote_count.push_back(j);
    }
    voted.clear();
    dead_player = *max_element(vote_count.begin(), vote_count.end());
    switchState();
    return dead_player;
}
int Logic::killedByShots()
{
    int m_count = 0;
    for (int i = 0; i < players.size(); i++)
    {
        if ((players[i].role < 0) && (players[i].is_dead == false))
            m_count++;
    }
    if ((shots.size() != m_count) || ((shots.size() > 1) && (ShotsEqual() == false)))
        dead_player = 0;
    else
        dead_player = shots[0];
    shots.clear();
    switchState();
    return dead_player;
}

int Logic::ItsCountingTime()
{
    if (daytime == true)
        return killedByVotes();
    return killedByShots();
}
QString Logic::parsedForVote(int num, QString speech)
{
    QString newSpeech;
    if((speech.contains("I put on vote player #")) && (s == DISCUSSION))
    {
        int vote = speech.at(speech.lastIndexOf("I put on vote player #")).digitValue();
        voted[vote] = 0;
    }
    newSpeech = "\nPlayer #" + QString::number(num) + ": " + speech + "\n";
    return newSpeech;
}

bool Logic::ShotsEqual()
{
    for (int i = 1; i < shots.size(); i++)
        if (shots[i-1] != shots[i])
            return false;
    return true;
}

void Logic::EasyNumber(QTcpSocket* q)
{
    if(!(numbers.empty()))
    {
        int number = numbers.back();
        numbers.pop_back();
        emit YourNumber(q, number);
    }
}

void Logic::switchState()
{
    if(s == FIRST_NIGHT)
    {
        s = DISCUSSION;
        daytime = true;
        for(int i = 0; i < players.size(); i++)
            if(players[i].is_dead == false)
            {
                emit PlayerSpeaking(players[i].number);
                break;
            }
     }
    else if(s == DISCUSSION)
    {
        if(voted.empty())
        {
            s = SHOOTING;
            daytime = false;
        }
        else
            s = VOTING;
    }
    else if (s == VOTING)
    {
        if (dead_player != 0)
        {
            s = DEAD_PLAYER;
            for(int i = 0; i < players.size(); i++)
            {
                if(players[i].number == dead_player)
                    players[i].is_dead = true;
            }
        }
        else
        {
            s = SHOOTING;
            daytime = false;
        }
    }
    else if (s == SHOOTING)
    {
        s = CHECKING;
    }
    else if (s == CHECKING)
    {
        if(dead_player != 0)
        {
            s = DEAD_PLAYER;
            for(int i = 0; i < players.size(); i++)
            {
                if(players[i].number == dead_player)
                    players[i].is_dead = true;
            }
        }
        else
        {
            s = DISCUSSION;
            daytime = true;
            for(int i = 0; i < players.size(); i++)
                if(players[i].is_dead == false)
                {
                    emit PlayerSpeaking(players[i].number);
                    break;
                }
        }
    }
    else if (s == DEAD_PLAYER)
    {
        checkForWinner();
        if(daytime == true)
        {
            s = SHOOTING;
            daytime = false;
        }
        else
        {
            s = DISCUSSION;
            daytime = true;
            for(int i = 0; i < players.size(); i++)
                if(players[i].is_dead == false)
                {
                    emit PlayerSpeaking(players[i].number);
                    break;
                }
        }

    }
}

void Logic::checkForWinner()
{
    int living_mafias = 0;
    int living_players = 0;
    for(int i = 0; i < players.size(); i++)
    {
        if(players[i].is_dead == false)
        {
            if(players[i].role < 0)
                living_mafias++;
            living_players++;
        }
    }
    if(living_mafias == 0)
        emit HereIsTheWinner("\nVictory! City has won.\n");
    else if(living_mafias * 2 == living_players)
        emit HereIsTheWinner("\nFailure. Mafia has won.\n");
}
