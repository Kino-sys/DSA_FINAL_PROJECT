#pragma once
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
using namespace std;

struct node{ //Node Creation
    string song;
    node* next;
    node* prev;
    node(string s){
        song = s;
        next = nullptr;
        prev = nullptr;
    }
};

class MusicPlayer{
public:
node* head;
node* current;
int size; 
atomic<bool> isPlaying;
thread playerThread;
string logMsg; // replaces cout for GUI

MusicPlayer(){
    head = nullptr;
    current = nullptr;
    size = 0;
    isPlaying = false;
    logMsg = "SYSTEM READY -- CIRCULAR DOUBLY LINKED LIST INITIALIZED";
}

// GUI version: takes song name as parameter instead of cin
void addMusic(const string& song){
    node* newNode = new node(song);
    if (head==nullptr){
        head = newNode;
        head->next = head;
        head->prev = head;
        current = head;    
    }
    else{
        node* tail = head->prev;
        tail->next = newNode;
        newNode->prev = tail;
        newNode->next = head;
        head->prev = newNode;
    }
    size++;
    logMsg = "ADDED: \"" + song + "\" -- NODE INSERTED AT TAIL";
}

void playLoop(){
    while(isPlaying){
        if(!current) break;
        string path = "C:\\Users\\DELL\\OneDrive\\Desktop\\DSA_FINAL_PROJECT\\Music\\" + current->song + ".wav";
        logMsg = "PLAYING: \"" + current->song + "\"";
        PlaySoundA(path.c_str(), NULL, SND_FILENAME | SND_ASYNC);
        while(isPlaying){
            Sleep(100);
            if(PlaySoundA(NULL, NULL, SND_NOSTOP | SND_ASYNC)){
                break;
            }
        }
        if(isPlaying)
            current = current->next;
    }
    logMsg = "PLAYBACK STOPPED";
}

// GUI version: takes index as parameter instead of cin
void deleteMusic(int i){
    if(!head){ logMsg = "PLAYLIST IS EMPTY"; return; }
    if(i<1 || i>size){ logMsg = "INVALID SONG NUMBER"; return; }
    node* temp = head;
    for(int j=1;j<i;j++) temp = temp->next;
    string name = temp->song;
    if(size == 1){
        delete temp;
        head = nullptr;
        current = nullptr;
        size = 0;
        logMsg = "DELETED: \"" + name + "\"";
        return;
    }
    if(temp == current) current = temp->next;
    if(temp == head) head = temp->next;
    temp->prev->next = temp->next;
    temp->next->prev = temp->prev;
    delete temp;
    size--;
    logMsg = "DELETED: \"" + name + "\"";
}

void play(){
    if(!head){ logMsg = "PLAYLIST IS EMPTY"; return; }
    if(!current) current = head;
    if(isPlaying){ logMsg = "ALREADY PLAYING"; return; }
    isPlaying = true;
    playerThread = thread(&MusicPlayer::playLoop, this);
    playerThread.detach();
}

void stopMusic(){
    isPlaying = false;
    PlaySoundA(NULL, NULL, 0);
    logMsg = "MUSIC STOPPED";
}

void next(){
    if(!current) return;
    bool wasPlaying = isPlaying;
    if(wasPlaying) stopMusic();
    current = current->next;
    logMsg = "CURRENT -> \"" + current->song + "\"";
    if(wasPlaying) play();
}

void prev(){
    if(!current) return;
    bool wasPlaying = isPlaying;
    if(wasPlaying) stopMusic();
    current = current->prev;
    logMsg = "CURRENT -> \"" + current->song + "\"";
    if(wasPlaying) play();
}

void randomSong(){
    if(!head) return;
    bool wasPlaying = isPlaying;
    if(wasPlaying) stopMusic();
    int idx = rand() % size;
    current = head;
    for(int i = 0; i < idx; i++) current = current->next;
    logMsg = "RANDOM PICK -> \"" + current->song + "\"";
    if(wasPlaying) play();
}

// GUI version: select by index instead of cin
void setByIndex(int index){
    if(index < 1 || index > size) return;
    current = head;
    for(int i = 1; i < index; i++) current = current->next;
    logMsg = "SELECTED: \"" + current->song + "\"";
}

// Returns list of songs + whether each is current (for GUI to render)
vector<pair<string,bool>> getSongs(){
    vector<pair<string,bool>> songs;
    if(!head) return songs;
    node* temp = head;
    do{
        songs.push_back({temp->song, temp == current});
        temp = temp->next;
    } while(temp != head);
    return songs;
}

~MusicPlayer(){
    stopMusic();
    if(!head) return;
    node* temp = head;
    do{
        node* next = temp->next;
        delete temp;
        temp = next;
    }
    while(temp != head);
}
};