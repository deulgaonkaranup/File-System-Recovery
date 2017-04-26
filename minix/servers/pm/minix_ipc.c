#include "minix_ipc.h"

void do_resetpage(void){
	lookup_index = 0;
}

int isValidTopic(const char *name){
	int i;
	for(i = 0; i < tList.index; i++){
		if(strcmp(name, tList.topics[i].name) == 0)
			return i;
	}
	return -1;
}

int do_topiclookup(char *names){
	
	int i;
	memset(names,0,MAX_TOPIC_LENGTH*MAX_TOPIC_PAGE_SIZE);
	if(lookup_index != 0)
		i = lookup_index;
	else
		i = 0;

	for(; (i < MAX_TOPIC_PAGE_SIZE && i < tList.index); i++){
		strcat(names,tList.topics[i].name);
		strcat(names,",");
	}
	names[strlen(names)] = '\0';
	if(i != tList.index){
		lookup_index = i + 1;
		return 1;
	}
	
	lookup_index = 0;
	return 0;
}

int do_createtopic(int id,const char *name){
	if(name == NULL || strlen(name) == 0)
		return INVALID_INPUT;
	
	if(isValidTopic(name) != -1)
		return DUPLICATE_TOPIC_FOUND;	
	
	lookup_index = 0;
	if(tList.index < MAX_TOPICS){
		strcpy(tList.topics[tList.index++].name,name);
		tList.topics[tList.index].owner_id = id;
		return 0;
	}

	return MAX_LIMIT_REACHED;
}

bool isRegisteredPub(int id,int tIndex,int *topic_index){

   if(publishers.index == 0)
      return false;
	int i,j;
   for(i = 0; i < publishers.index; i++){
      if(publishers.pList[i].topic_index == tIndex){
			*topic_index = i;
         for(j = 0; j < publishers.pList[i].index; j++){
            if(publishers.pList[i].users[j] == id){
               return true;
            }
         }
         return false;
      }
   }
   return false;
}

bool isRegisteredSubs(int id,int tIndex,int *topic_index){

	if(subscribers.index == 0)
		return false;
	int i,j;
	for(i = 0; i < subscribers.index; i++){
		if(subscribers.sList[i].topic_index == tIndex){
			*topic_index = i;
			for(j = 0; j < subscribers.sList[i].index; j++){
				if(subscribers.sList[i].users[j] == id){
					return true;
				}
			}
			return false;
		}		
	}
	return false;
}

int do_tsubscriber(int id,const char *name){
	
	if(name == NULL || strlen(name) == 0)
		return INVALID_INPUT;

	int index = isValidTopic(name);
	if(index == -1)
		return TOPIC_NOT_FOUND;
	
	lookup_index = 0;	
	int tIndex = -1;
	if(isRegisteredSubs(id,index,&tIndex))
		return 0;
	
	if(tIndex == -1){
		subscribers.sList[subscribers.index].topic_index = index;
		subscribers.sList[subscribers.index].users[subscribers.sList[subscribers.index].index++] = id;
		subscribers.index++;
		return 0;
	}
	
	if(subscribers.sList[tIndex].index == MAX_SUBSCRIBERS_PER_TOPIC)
		return MAX_LIMIT_REACHED;
	
	subscribers.sList[tIndex].users[subscribers.sList[tIndex].index++] = id;
	return 0;
}

void getSubscriberIDs(int tindex,int mlist_index,int msgs_index){
	int i;
	for(i = 0; i < subscribers.index; i++){
		if(subscribers.sList[i].topic_index == tindex){
			int j;
			for(j = 0; j < subscribers.sList[i].index; j++){
				msgList.mList[mlist_index].messages[msgs_index].subscb_users[j] = subscribers.sList[i].users[j];
			}
			msgList.mList[mlist_index].messages[msgs_index].count = subscribers.sList[i].index;
		}
	}
}

int getTopicIndexMessageList(int index){
	int i;
	for(i = 0; i < msgList.index; i++){
		if(msgList.mList[i].topic_index == index)
			return i;
	}
	return -1;
}

int do_publish(char *tname,int id,char *message){
	
	if(message == NULL || strlen(message) == 0)
		return INVALID_INPUT;
	
	if(tname == NULL || strlen(tname) == 0)
		return INVALID_INPUT;
	
	int topic_index = isValidTopic(tname);	
	if(topic_index == -1)
		return TOPIC_NOT_FOUND;

	int publisher_index = -1;
	if(!isRegisteredPub(id,topic_index,&publisher_index)) 
		return USER_NOT_REGISTERED;
	
	lookup_index = 0;		
	int msg_index = getTopicIndexMessageList(topic_index);
	if(msg_index == -1){
		if(msgList.index == MAX_TOPICS)
			return MAX_LIMIT_REACHED;

		strcpy(msgList.mList[msgList.index].messages[0].message,message);
		int len = strlen(msgList.mList[msgList.index].messages[0].message);
		msgList.mList[msgList.index].messages[0].message[len] = '\0';
		msgList.mList[msgList.index].index++;
		msgList.mList[msgList.index].topic_index = topic_index;
		msgList.mList[msgList.index].messages[0].deleted = false;
		getSubscriberIDs(topic_index,msgList.index,0);
		msgList.index++;
		return 0;
	}else{
		if(msgList.mList[msg_index].index == MAX_MESSAGES_PER_TOPIC){
			int j;
			for(j = 0; j < msgList.mList[msg_index].index; j++){
				if(msgList.mList[msg_index].messages[j].deleted == true){
					memset(msgList.mList[msg_index].messages[j].message,0,MAX_MESSAGE_LENGTH);
					memset(msgList.mList[msg_index].messages[j].subscb_users,0,MAX_SUBSCRIBERS_PER_TOPIC);
					strcpy(msgList.mList[msg_index].messages[j].message,message);
					int len = strlen(msgList.mList[msg_index].messages[j].message);
					msgList.mList[msg_index].messages[j].message[len] = '\0';
					getSubscriberIDs(topic_index,msg_index, j);
					msgList.mList[msg_index].messages[j].deleted = false;
					return 0;
				}
			}
			return MAX_LIMIT_REACHED;
		}
		int index = msgList.mList[msg_index].index;
		memset(msgList.mList[msg_index].messages[index].message,0,MAX_MESSAGE_LENGTH);
		memset(msgList.mList[msg_index].messages[index].subscb_users,0,MAX_SUBSCRIBERS_PER_TOPIC);
		strcpy(msgList.mList[msg_index].messages[index].message,message);
		int len = strlen(msgList.mList[msg_index].messages[index].message);
		msgList.mList[msg_index].messages[index].message[len] = '\0';
		getSubscriberIDs(topic_index,msg_index, index);
		msgList.mList[msg_index].messages[index].deleted = false;
		msgList.mList[msg_index].index++;
	}
	return 0;
}

bool _getMessage_(int index,int id,char *rmessage){
	
	int mIndex = -1;
	int i;
	for(i = 0; i < msgList.index; i++){
		if(msgList.mList[i].topic_index == index){
			mIndex = i;
			break;
		}
	}
	int count;
	
	if(msgList.mList[mIndex].index == 0)
		return false;
	
	struct _sys_message_internal msgInternal;
	count = msgList.mList[mIndex].index;
	int j;
	
	for(i = 0; i < count; i++){
		if(msgList.mList[mIndex].messages[i].deleted != true){
			msgInternal = msgList.mList[mIndex].messages[i];
			for(j = 0; j < msgInternal.count; j++){
				if(id == msgInternal.subscb_users[j]){
					strcpy(rmessage, msgInternal.message);
					rmessage[strlen(rmessage)] = '\0';
					msgInternal.subscb_users[j] = -1;
					bool flag = false;
					int k;
					for(k = 0; k < msgInternal.count;k++){
						if(msgInternal.subscb_users[k] != -1){
							flag = true;
							break;
						}
					}
					if(!flag)
						msgList.mList[mIndex].messages[i].deleted = true;
					return true;
				}
			}
		}
	}
	return false;
}

int do_getmessage(char *tname,int id,char *rmsg){
	
	if(rmsg == NULL)
		return INVALID_INPUT;
	
	if(tname == NULL || strlen(tname) == 0)
		return INVALID_INPUT;
	
	int topic_index = isValidTopic(tname);
	if(topic_index == -1)
		return TOPIC_NOT_FOUND;

	memset(rmsg,0,MAX_MESSAGE_LENGTH);
	int subs_index = -1;
	if(!isRegisteredSubs(id,topic_index,&subs_index))
		return USER_NOT_REGISTERED;
	
	lookup_index = 0;
	char msg[MAX_MESSAGE_LENGTH];
	if(!_getMessage_(topic_index,id,msg))
		return NO_MORE_MESSAGES;
	
	strcpy(rmsg,msg);
	rmsg[strlen(rmsg)] = '\0';
	
	return 0;
}

int do_tpublisher(int id,char *name){
	
	if(name == NULL || strlen(name) == 0)
		return INVALID_INPUT;

	int index = isValidTopic(name);
	if(index == -1)
		return TOPIC_NOT_FOUND;
	
	lookup_index = 0;
	int tIndex = -1;
	if(isRegisteredPub(id,index,&tIndex))
		return 0;

	if(tIndex == -1){
		publishers.pList[publishers.index].topic_index = index;
		publishers.pList[publishers.index].users[publishers.pList[publishers.index].index++] = id;
		publishers.index++;
		return 0;
	}
	if(publishers.pList[tIndex].index == MAX_PUBLISHERS_PER_TOPIC)
		return MAX_LIMIT_REACHED;
	
	publishers.pList[tIndex].users[publishers.pList[tIndex].index++] = id;
	return 0;
}
