#ifndef AT_LISTENER_H
#define AT_LISTENER_H

typedef void ( *Callback ) ( void *, char * );

typedef struct
{
    char *response;
    Callback callback;

}
ModemResponseEntry;

#define MODEM_RESPONSE_ENTRY_INIT() {\
  .response=NULL, \
  .callback=NULL, \
}

typedef struct
{
    ModemResponseEntry *entries;
    int numberOfEntries;
}
ModemResponseTable;

#define MODEM_RESPONSE_TABLE_INIT() {\
  .entries=NULL, \
  .numberOfEntries=0, \
}

#define MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(a) {\
  .entries=a, \
  .numberOfEntries=(sizeof(a)/sizeof(ModemResponseEntry)), \
}

typedef struct
{
    char *cmd;
    ModemResponseTable *pResponseTable;
    int secTimeout;
    int freeCmd; //Indicates that the cmd pointer was dynamically allocated
}
ModemCommand;

#define MODEM_COMMAND_INIT(a,b,c) {\
  .cmd=a, \
  .pResponseTable=b, \
  .secTimeout=c, \
  .freeCmd=0, \
}

#define MODEM_COMMAND_DEFAULT_INIT() MODEM_COMMAND_INIT(NULL,NULL,60)

#endif //AT_LISTENER_H
