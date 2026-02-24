#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <SFML/Audio.hpp>
using namespace std;

struct node{
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
    atomic<bool> isPaused;
    string logMsg;

    sf::Music music;  // SFML music object — handles playback, seek, volume

    MusicPlayer(){
        head = nullptr;
        current = nullptr;
        size = 0;
        isPlaying = false;
        isPaused = false;
        logMsg = "SYSTEM READY -- CIRCULAR DOUBLY LINKED LIST INITIALIZED";
        music.setVolume(70); // default volume
    }

    void addMusic(const string& song){
        node* newNode = new node(song);
        if(head == nullptr){
            head = newNode;
            head->next = head;
            head->prev = head;
            current = head;
        } else {
            node* tail = head->prev;
            tail->next = newNode;
            newNode->prev = tail;
            newNode->next = head;
            head->prev = newNode;
        }
        size++;
        logMsg = "ADDED: \"" + song + "\" -- NODE INSERTED AT TAIL";
    }

    void deleteMusic(int i){
        if(!head){ logMsg = "PLAYLIST IS EMPTY"; return; }
        if(i<1 || i>size){ logMsg = "INVALID SONG NUMBER"; return; }
        node* temp = head;
        for(int j=1; j<i; j++) temp = temp->next;
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

    // Load and play current song via SFML
    bool loadAndPlay(){
        if(!current) return false;
        string path = "C:\\Users\\DELL\\OneDrive\\Desktop\\DSA_FINAL_PROJECT\\Music\\" + current->song + ".wav";
        if(!music.openFromFile(path)){
            logMsg = "ERROR: Could not open \"" + current->song + "\"";
            return false;
        }
        music.play();
        logMsg = "PLAYING: \"" + current->song + "\"";
        return true;
    }

    void play(){
        if(!head){ logMsg = "PLAYLIST IS EMPTY"; return; }
        if(!current) current = head;
        if(isPlaying && !isPaused){ logMsg = "ALREADY PLAYING"; return; }
        isPaused = false;
        isPlaying = true;
        loadAndPlay();
    }

    void pause(){
        if(!isPlaying){ logMsg = "NOTHING IS PLAYING"; return; }
        if(isPaused){ logMsg = "ALREADY PAUSED"; return; }
        isPaused = true;
        music.pause();
        logMsg = "PAUSED: \"" + (current ? current->song : "") + "\"";
    }

    void resume(){
        if(!isPlaying){ logMsg = "NOTHING IS PLAYING"; return; }
        if(!isPaused){ logMsg = "NOT PAUSED"; return; }
        isPaused = false;
        music.play();
        logMsg = "RESUMED: \"" + (current ? current->song : "") + "\"";
    }

    void stopMusic(){
        isPlaying = false;
        isPaused = false;
        music.stop();
        logMsg = "MUSIC STOPPED";
    }

    void next(){
        if(!current) return;
        bool wasPlaying = isPlaying;
        music.stop();
        current = current->next;
        isPlaying = false;
        isPaused = false;
        logMsg = "NEXT -> \"" + current->song + "\"";
        if(wasPlaying) play();
    }

    void prev(){
        if(!current) return;
        bool wasPlaying = isPlaying;
        music.stop();
        current = current->prev;
        isPlaying = false;
        isPaused = false;
        logMsg = "PREV -> \"" + current->song + "\"";
        if(wasPlaying) play();
    }

    void randomSong(){
        if(!head) return;
        bool wasPlaying = isPlaying;
        music.stop();
        int idx = rand() % size;
        current = head;
        for(int i=0; i<idx; i++) current = current->next;
        isPlaying = false;
        isPaused = false;
        logMsg = "RANDOM PICK -> \"" + current->song + "\"";
        if(wasPlaying) play();
    }

    void setByIndex(int index){
        if(index<1 || index>size) return;
        current = head;
        for(int i=1; i<index; i++) current = current->next;
        logMsg = "SELECTED: \"" + current->song + "\"";
    }

    
    void seek(float delta){
        if(!isPlaying || music.getDuration().asSeconds() == 0) return;
        float total = music.getDuration().asSeconds();
        float cur = music.getPlayingOffset().asSeconds();
        float newPos = cur + delta * total * 0.1f; 
        music.setPlayingOffset(sf::seconds(newPos));
        logMsg = "SEEKED TO " + to_string((int)newPos) + "s";
    }

    // Returns 0.0 to 1.0 real progress
    float getProgress(){
        if(!isPlaying || music.getDuration().asSeconds() == 0) return 0.0f;
        return music.getPlayingOffset().asSeconds() / music.getDuration().asSeconds();
    }

    string getTimeElapsed(){
        int s = (int)music.getPlayingOffset().asSeconds();
        return to_string(s/60) + ":" + (s%60<10?"0":"") + to_string(s%60);
    }

    string getTimeDuration(){
        int s = (int)music.getDuration().asSeconds();
        if(s == 0) return "0:00";
        return to_string(s/60) + ":" + (s%60<10?"0":"") + to_string(s%60);
    }

    // Volume 0-100
    void setVolume(float v){
        v = max(0.0f, min(100.0f, v));
        music.setVolume(v);
    }

    float getVolume(){
        return music.getVolume();
    }

    // Call every frame — auto advance when song ends
    void update(){
        if(isPlaying && !isPaused){
            if(music.getStatus() == sf::SoundSource::Status::Stopped){
                current = current->next;
                loadAndPlay();
            }
        }
    }

    vector<pair<string,bool>> getSongs(){
        vector<pair<string,bool>> songs;
        if(!head) return songs;
        node* temp = head;
        do{
            songs.push_back({temp->song, temp==current});
            temp = temp->next;
        } while(temp != head);
        return songs;
    }

    ~MusicPlayer(){
        music.stop();
        if(!head) return;
        node* temp = head;
        do{
            node* next = temp->next;
            delete temp;
            temp = next;
        } while(temp != head);
    }
};