#ifndef PLAYER_H
#define PLAYER_H

struct Player
{
    int role = 0, number = 0;
    bool my_speech = false;
    bool has_voted = false;
    bool is_dead = false;
};

#endif // PLAYER_H
