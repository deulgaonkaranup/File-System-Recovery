#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define true 1
#define false 0

#define MAX_TOPICS 5
#define MAX_MESSAGES_PER_TOPIC 5
#define MAX_SUBSCRIBERS_PER_TOPIC 5
#define MAX_PUBLISHERS_PER_TOPIC 5
#define MAX_TOPIC_LENGTH 1024
#define MAX_MESSAGE_LENGTH 1024
#define MAX_TOPIC_PAGE_SIZE 5

#define MAX_LIMIT_REACHED 101
#define TOPIC_NOT_FOUND 102
#define USER_NOT_REGISTERED 103
#define NO_MORE_MESSAGES 104
#define INVALID_INPUT 105
#define DUPLICATE_TOPIC_FOUND 106

typedef int bool;

struct _sys_topic{
	char name[MAX_TOPIC_LENGTH];
	int owner_id;
};

struct _sys_topic_list{
	int index;
	struct _sys_topic topics[MAX_TOPICS];
};

struct _sys_msg_usr{
	int proc_id;
};

struct _sys_subscribers{	
	int topic_index;
	int users[MAX_SUBSCRIBERS_PER_TOPIC];
	int index;
};

struct _sys_subscribers_list{
	struct _sys_subscribers sList[MAX_TOPICS];
	int index;
};

struct _sys_publishers{
	int topic_index;
	int users[MAX_PUBLISHERS_PER_TOPIC];
	int index;
};

struct _sys_publishers_list{
	struct _sys_publishers pList[MAX_TOPICS];
	int index;
};

struct _sys_message_internal{
	char message[MAX_MESSAGE_LENGTH];
	int subscb_users[MAX_SUBSCRIBERS_PER_TOPIC];
	int count;
	bool deleted;
};

struct _sys_message{
	int topic_index;
	struct _sys_message_internal messages[MAX_MESSAGES_PER_TOPIC];
	int index;
};

struct _sys_message_list{
	struct _sys_message mList[MAX_TOPICS];
	int index;
};

/* private */
int isValidTopic(const char *name);
bool isRegisteredPub(int id,int tIndex,int *topic_index);
bool isRegisteredSubs(int id,int tIndex,int *topic_index);
void getSubscriberIDs(int tindex,int mlist_index,int msg_index);
int getTopicIndexMessageList(int index);
bool _getMessage_(int index,int id,char *rmessage);

/* public Variables */
struct _sys_topic_list tList;
struct _sys_subscribers_list subscribers;
struct _sys_publishers_list publishers;
struct _sys_message_list msgList;
int lookup_index;

/* public */
void do_resetpage(void);
int do_topiclookup(char *names);
int do_createtopic(int id,const char *name);
int do_tsubscriber(int id,const char *name);
int do_publish(char *tname,int id,char *message);
int do_getmessage(char *tname,int id,char *message);
int do_tpublisher(int id,char *name);
