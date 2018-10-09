#ifndef ATYPES_H
#define ATYPES_H


typedef enum command
{
    INI = 0,
    ACK,
    ADD,
    LIST,
    MOVING,
    DONE
} command_t;  


typedef struct node
{
    char *ip;
    int port;
    int delay;
    char *name;
} node_t;

#endif
