//UDPServer.c
 
/*
 *  gcc -o server UDPServer.c
 *  ./server
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "duckchat.h"
//defined
#define BUFLEN 1024
using namespace std;
//globals
struct sockaddr recAddr;
socklen_t fromlen = sizeof(recAddr);
int sockfd;
struct addrinfo *addrAr;
map<string, string> addrToUser;
map<string, string> userToAddr;
map<string,vector<string> > usrLisChan;
map<string,vector<string> > usrTlkChan;
map<string,vector<string> > chanTlkUser;
vector<string> channels;
//methods
int connectToSocket(char*, char*);
void err(char*);
int readRequestType(struct request*, int);
int sayReq(struct request_say*);
int checkValidAddr();
string getUserOfCurrAddr();
string getAddr_string();

//program
int main(int argc, char **argv)
{
    addrAr = NULL;
    sockfd = 0;
    connectToSocket(argv[1], argv[2]);
    while(1)
    {
        //print stuff
        map<string,string>::iterator it;
        if(!addrToUser.empty()) {
            cout << "SIZE OF AtoU: " << addrToUser.size() << "\n";
            for(it = addrToUser.begin(); it != addrToUser.end(); it++) {
                        cout << it->first << " is the address.\n";
                        cout << it->second << " is the user.\n";
            }  
        }
        map<string,string>::iterator its;
        if(!userToAddr.empty()) {
            cout << "SIZE OF UtoA: " << userToAddr.size() << "\n";
            for(its = userToAddr.begin(); its != userToAddr.end(); its++) {
                cout << its->first << " is the user.\n";
                cout << its->second << " is the addrr.\n";
            }
        } 
        for(int i=0; i<channels.size(); i++) {
            vector<string> uOnC = chanTlkUser[channels[i]];
            if(!uOnC.empty()) {
                for(int j=0; j<uOnC.size(); j++) {
                    cout << uOnC[j] << " is user on channel: " << channels[i] << " from chanTlkUser.\n";
                }
            }
            chanTlkUser[channels[i]] = uOnC;
        }   
        //for multiple requests maybe
        // requests = (struct request*) malloc(sizeof (struct request) + BUFLEN); 
        char *buf = new char[BUFLEN];
        struct request *requests = (struct request*)malloc(sizeof(struct request*) + BUFLEN);  
        int bal = 0;
        bal = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr*)&recAddr, &fromlen);
        if(bal > 0) {
            printf("recv()'d %d bytes of data in buf\n", bal);
            requests = (request*) buf;
            readRequestType(requests, bal);  
            //print stuff
            map<string,string>::iterator it;
            if(!addrToUser.empty()) {
                cout << "SIZE OF AtoU: " << addrToUser.size() << "\n";
                for(it = addrToUser.begin(); it != addrToUser.end(); it++) {
                            cout << it->first << " is the address.\n";
                            cout << it->second << " is the user.\n";
                }  
            }
            map<string,string>::iterator its;
            if(!userToAddr.empty()) {
                cout << "SIZE OF UtoA: " << userToAddr.size() << "\n";
                for(its = userToAddr.begin(); its != userToAddr.end(); its++) {
                    cout << its->first << " is the user.\n";
                    cout << its->second << " is the addrr.\n";
                }
            } 
            for(int i=0; i<channels.size(); i++) {
                vector<string> uOnC = chanTlkUser[channels[i]];
                if(!uOnC.empty()) {
                    for(int j=0; j<uOnC.size(); j++) {
                        cout << uOnC[j] << " is user on channel: " << channels[i] << " from chanTlkUser.\n";
                    }
                }
                chanTlkUser[channels[i]] = uOnC;
            }   
        } 
       requests = NULL;
       delete[] buf;   
    }
    return 0;
}
//returns string of username of current request address
string getUserOfCurrAddr()
{ 
    string realAddrString = getAddr_string();
    string aTmp = addrToUser[realAddrString];
    if(aTmp == "") {
        return "empty";
    }
    return aTmp;
}
//returns string form of address
string getAddr_string() {
    //new request address info
    struct sockaddr_in* address = (struct sockaddr_in*)&recAddr;   
    char *addrString = (char*)malloc(sizeof(char)*BUFLEN);
    //make address string
    inet_ntop(AF_INET, &(address->sin_addr), addrString, BUFLEN);
    //have tmp var
    string realAddrString = addrString;
    free (addrString);
    return realAddrString;
}
//check if current request address is valid or exist in map
int checkValidAddr(struct request *r) 
{
    //OLD CODE //new request address info
    // struct sockaddr_in* address = (struct sockaddr_in*)&recAddr;   
    // char *addrString = (char*)malloc(sizeof(char)*BUFLEN);
    // //make address string
    // inet_ntop(AF_INET, &(address->sin_addr), addrString, BUFLEN);
    // //have tmp var
    string realAddrString = getAddr_string();
    //free (addrString);
    //look in map for address
    //string aTmp = addrToUser[realAddrString];
    map<string,string>::iterator it = addrToUser.find(realAddrString);
    if(it == addrToUser.end()) {
        cout << "super baddd addressss mann\n";
        cout << realAddrString << " that THING\n";
        return -1;
    } 
    return 0;
}
//for errors
void err(char *str)
{
    perror(str);
    exit(1);
}
//handle say requests
int sayReq(struct request_say *rs)
{
    /*
    get username of request
    get channel of request*/
    string channel = rs->req_channel;
    string message = rs->req_text;
    string username = getUserOfCurrAddr();
    cout << "this is username in sayReq " << username << "\n";
    //get list of users on channel from usrLisChan
    //vector<string> tmpU = usrLisChan[username];
    map<string,vector<string> >::iterator hit = chanTlkUser.find(channel);
    if(hit == chanTlkUser.end()) {
        cout << "HOLD not here : " << channel <<"\n";  
        return -1; 
    }
    vector<string> tmpU = hit->second;
    //for all users on the channel
        //write the message to those users by there address
    for(int i=0; i<tmpU.size(); i++) {
        cout << "user: " << tmpU[i] << " on channel: " << channel << "\n";
        //get address of current user
        struct sockaddr_in address;
        string ad = userToAddr[tmpU[i]];
        char *s= (char*) malloc(sizeof(char)*BUFLEN);
        //move ad to t (address)
        strncpy(s, ad.c_str(), strlen(ad.c_str()));
        //from s to address and format
        inet_pton(AF_INET, s, &(address.sin_addr));
        //setup message to send
        struct text_say *msg= (struct text_say*) malloc(sizeof(struct text_say));
        //set message type
        msg->txt_type = htonl(TXT_SAY);
        //add username (from) and message 
        strncpy(msg->txt_username, username.c_str(), USERNAME_MAX);
        strncpy(msg->txt_text, message.c_str(), SAY_MAX);
        strncpy(msg->txt_channel, channel.c_str(), CHANNEL_MAX);
        //send message
        size_t size = sizeof(struct sockaddr_in);
        int res= sendto(sockfd, msg, sizeof(struct text_say), 0, (struct sockaddr*)&address, size);
        if (res == -1) {
            cout << "sendto very badd \n";
            return -1;
        }
        free(msg);
    }
    tmpU.clear();
    return 0;
}
//handle login requests
int loginReq(struct request_login *rl)
{
    string realAddrString = getAddr_string();

    //username
    string username = rl->req_username;
    cout << "this is the real addr string in login: " << realAddrString << "\n";

    cout << "username in login req: " << username << "\n";
    cout << "address in login req: " << realAddrString << "\n";
    //add address and username to map
    addrToUser.insert(pair<string, string>(realAddrString, username));
    userToAddr.insert(pair<string, string>(username, realAddrString));
    //add user to common
    map<string,vector<string> >::iterator it = chanTlkUser.find("Common");
    vector<string> usersC;
    if(it == chanTlkUser.end()) {
        chanTlkUser.insert(pair<string,vector<string> >("Common", usersC));
    }
    it = chanTlkUser.find("Common");
    usersC = it->second;
    usersC.insert(usersC.begin(), username);
    chanTlkUser["Common"] = usersC;
    //add to user lisChan
    vector<string> chans;
    chans.insert(chans.begin(), "Common");
    usrLisChan.insert(pair<string,vector<string> >(username, chans));
    usrTlkChan.insert(pair<string,vector<string> >(username, chans));
    // OLD CODE//new request address inf.find()o
    // struct sockaddr_in* address = (struct sockaddr_in*)&recAddr;
    
    // char *addrString = (char*)malloc(sizeof(char)*BUFLEN);
    // //make address string
    // inet_ntop(AF_INET, &(address->sin_addr), addrString, BUFLEN);
    // //this is our readable address
    // string realAddrString = addrString;
    
    
    //free (addrString);
    //look for address in addrToUser
    //__OLD_CODE__ map<string, string>::iterator hit = addrToUser.find(realAddrString);
    // if(hit != addrToUser.end()) {
    //     addrToUser.erase(realAddrString);
    // }
    // // look for use in userToAddr
    // hit = userToAddr.find(username);
    // if(hit != userToAddr.end()) {
    //     userToAddr.erase(username);
    // }
    // // look for users in channel listen usrLisChan
    // map<string, vector<string> >::iterator git = usrLisChan.find(username);
    // if(git != usrLisChan.end()) {
    //     usrLisChan.erase(username);
    // }
    // //look for users in channel talk usrTlkChan
    // git = usrTlkChan.find(username);
    // if(git != usrTlkChan.end()) {
    //     usrTlkChan.erase(username);
    // }
    //for all channels, if there is a user in the channels talk user list chanTlkUser[i] then erase the user from the chanTlkUser and add it back to channels talk user list chanTlkUser[i]
    // for(int i=0; i<channels.size(); i++) {
    //     vector<string> uOnC = chanTlkUser[channels[i]];
    //     if(!uOnC.empty()) {
    //         for(int j=0; j<uOnC.size(); j++) {
    //             if(username == uOnC[j]) {
    //                 uOnC.erase(uOnC.begin()+j);
    //             }
    //         }
    //     }
    //     chanTlkUser[channels[i]] = uOnC;
    // }
    // addrToUser[realAddrString] = username;
    // userToAddr[username] = realAddrString;
    // map<string,string>::iterator it = addrToUser.find(realAddrString);
    // if(it != addrToUser.end()) {
        
    //     cout << it->first << " that THING is in LoGIN " << it->second << " so is that\n";
    //     //return -1;
    // } else {
    //     cout << "super baddd addressss mann\n";
    // }
    return 0;
}
//handle login requests
int logoutReq(struct request_logout *rl)
{
    //new request address info
    cout << "Logout REquesssttt \n";
    //have tmp var
    string realAddrString = getAddr_string();
    string username = getUserOfCurrAddr();
    map<string,string>::iterator it;
    //delete address and user in both maps and both channe maps
    //look for address
    map<string, string>::iterator hit = addrToUser.find(realAddrString);
    if(hit != addrToUser.end()) {
        addrToUser.erase(realAddrString);
    }
    // look for use
    hit = userToAddr.find(username);
    if(hit != userToAddr.end()) {
        userToAddr.erase(username);
    }
    // look for user in channel listen
    map<string, vector<string> >::iterator git = usrLisChan.find(username);
    if(git != usrLisChan.end()) {
        usrLisChan.erase(username);
    }
    //look for user in channel talk
    git = usrTlkChan.find(username);
    if(git != usrTlkChan.end()) {
        usrTlkChan.erase(username);
    }
    //erase from chanTlkUser
    for(int i=0; i<channels.size(); i++) {
        map<string,vector<string> >::iterator it = chanTlkUser.find(channels[i]);
        vector<string> usersC = it->second;
        vector<string>::iterator vt = usersC.find(username);
        if(vt != usersC.end()) {
            usersC.erase(vt);
        }
    }
    //delete user and channel stuff
    //it = usrLisChan.find(user);
    //usrLisChan.erase(it);
    //it = usrTlkChan.find(user);
    //usrTlkChan.erase(it);
    return 0;
}
//handle login requests
int joinReq(struct request_join *rj)
{
    //create tmp vars for username and channel of request
    string chan = (string)rj->req_channel;

    string user = getUserOfCurrAddr();
    int trig = 0;
    map<string,vector<string> >::iterator it = chanTlkUser.find(chan);
    vector<string> usersC;
    if(it == chanTlkUser.end()) {
        //NEW CHANNEL
        //chanTlkUser.insert(pair<string,vector<string> >("Common", usersC));
        usersC.insert(usersC.begin(), user);
        chanTlkUser.insert(pair<string,vector<string> >(chan, usersC));
        channels.push_back(chan);

    } else {
        //old channel
        it = chanTlkUser.find(chan);
        usersC = it->second;
        for(int i=0; i<usersC.size(); i++) {
            if(usersC[i] == user) {
                cout << "User exists in channel already!!!!!!!!!!__\n";
                return -1;
            }
        }
        usersC.insert(usersC.begin(), user);
        chanTlkUser[chan] = usersC;
    }
    //tmp vector for channel user is listening to
    vector<string> chanList = usrLisChan[user];
    //tmp string for channel user is talking to
    vector<string> chanTlk = usrTlkChan[user];
    //tmp vec for users on channel
    //add new channel to back
    chanList.push_back(chan);
    //add channel to most recent channel to back
    chanTlk.push_back(chan);
    //
    //add vectors back to map, new channel at the back.
    usrLisChan[user] = chanList;
    usrTlkChan[user] = chanTlk;
    chanList.clear();
    chanTlk.clear();
    return 0;
}
//handle login requests
int leaveReq(struct request_leave *rl)
{
    return 0;
}
//handle login requests
int listReq(struct request_list *rl)
{
    return 0;
}
//handle login requests
int whoReq(struct request_who *rw)
{
    return 0;
}

int readRequestType(struct request *r, int b) 
{
    int fin = 0;
    int netHost = 0;
    netHost = ntohl(r->req_type);
    //check if addres is a crazy number or normal
    if(netHost > 6 || netHost < 0) {
       netHost = r->req_type;
    }
    //check if request address is valid
    cout << netHost << " this is the request type!!!! \n";
    if(netHost != 0) {
        if(checkValidAddr(r) == -1) {
            //bad address, return
            cout << "invalid address\n";
            return -1;
        } 
    }
    switch(netHost) {
    //printf("the value isss: %s \n", ntohl(r->req_type));
        case REQ_LOGIN:
            if(sizeof(struct request_login) == b) {
                cout << "login request\n";
                fin = loginReq((struct request_login*) r);
                break;
            } else {
                cout << "login request FAILED\n";
                break;
            } 
        case REQ_LOGOUT:
            if(sizeof(struct request_logout) == b) {
                cout << "logout request\n";
                fin = logoutReq((struct request_logout*) r);
                break;
            } else {
                cout << "logout request bad size\n";
                break;
            }   
        case REQ_JOIN:
            //printf("join case made \n");
            if(sizeof(struct request_join) == b) {
                cout << "join request\n";
                fin = joinReq((struct request_join*) r);
                break;
            } else {
                cout << "switch request bad size\n";
                break;
            }      
        case REQ_LEAVE:
            if(sizeof(struct request_leave) == b) {
                cout << "leave request\n";
                fin = leaveReq((struct request_leave*) r);
                break;
            } else {
                cout << "leave request bad size\n";
                break;
            }
        case REQ_SAY:
            if(sizeof(struct request_say) == b) {
                cout << "say request\n";
                fin = sayReq((struct request_say*) r);
                break;
            } else {
                cout << "say request bad size\n";
                break;
            }
        case REQ_LIST:
            if(sizeof(struct request_list) == b) {
                cout << "list request\n";
                fin = listReq((struct request_list*) r);
                break;
            } else {
                cout << "list request bad size\n";
                break;
            }
        case REQ_WHO:
            if(sizeof(struct request_who) == b) {
                cout << "who request\n";
                fin = whoReq((struct request_who*) r);
                break;
            } else {
                cout << "who request bad size\n";
                break;
            }
        default:
            cout << "default case hit!!\n";
    }
    return fin;
}

int connectToSocket(char* ip, char* port)
{
    struct addrinfo addressTmp;
    memset(&addressTmp, 0, sizeof addressTmp);
    addressTmp.ai_family = AF_INET;
    addressTmp.ai_socktype = SOCK_DGRAM;
    addressTmp.ai_flags = AI_PASSIVE;
    int check = 0;
    if((check = getaddrinfo(ip, port, &addressTmp, &addrAr))!= 0)
    {
        cout << "Server : getaddrinfo() NOT successful \n";
        return false;
    }
    if((sockfd = socket(addrAr->ai_family, addrAr->ai_socktype, addrAr->ai_protocol)) == -1)
    {
        cout << "Server : socket() NOT successful \n";
        return false;
    }
    if(bind(sockfd, addrAr->ai_addr, addrAr->ai_addrlen) == -1)
    {
        cout << "Server : bind() NOT successful \n";
        return false;
    }
    cout << "socket and bind successful! \n";
    return true;
}