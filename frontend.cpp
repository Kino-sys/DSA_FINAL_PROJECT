#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include "raylib.h"
#include "musicplayer.h"  
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std;


//Colours
#define COL_BG        Color{10,  10,  10,  255}
#define COL_SURFACE   Color{17,  17,  17,  255}
#define COL_CARD      Color{22,  22,  22,  255}
#define COL_BORDER    Color{42,  42,  42,  255}
#define COL_ACCENT    Color{200, 245, 66,  255}
#define COL_ACCENT2   Color{245, 66,  78,  255}
#define COL_TEXT      Color{240, 240, 240, 255}
#define COL_MUTED     Color{85,  85,  85,  255}
#define COL_YELLOW    Color{255, 220, 50,  255}
#define COL_BLACK     Color{0,   0,   0,   255}
#define COL_ACTIVE_BG Color{200, 245, 66,  20}

//Ui Const
const int SW = 480;
const int SH = 700;
const int PX = 20;
const int PW = SW - PX*2;
const int PLAYLIST_ROWS = 7;
float smoothScroll = 0;


bool IsMouseOver(int x, int y, int w, int h){
    Vector2 m = GetMousePosition();
    return m.x>=x && m.x<=x+w && m.y>=y && m.y<=y+h;
}

bool IsClicked(int x, int y, int w, int h){
    return IsMouseOver(x,y,w,h) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

bool DrawButton(int x, int y, int w, int h, const char* label, Color bg, Color fg, int fontSize=14){
    bool hover = IsMouseOver(x,y,w,h);
    bool clicked = IsClicked(x,y,w,h);
    Color drawBg = bg;
    if(hover){
        drawBg = {
            (unsigned char)min(255, bg.r+30),
            (unsigned char)min(255, bg.g+30),
            (unsigned char)min(255, bg.b+30),
            255
        };
    }
    DrawRectangle(x,y,w,h,drawBg);
    DrawRectangleLines(x,y,w,h, hover ? COL_ACCENT : COL_BORDER);
    int tw = MeasureText(label, fontSize);
    DrawText(label, x+(w-tw)/2, y+(h-fontSize)/2, fontSize, fg);
    if(hover) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    return clicked;
}

void DrawPanel(int x, int y, int w, int h, Color bg, Color border){
    DrawRectangle(x,y,w,h,bg);
    DrawRectangleLines(x,y,w,h,border);
}

string TruncateText(const string& text, int maxWidth, int fontSize){
    if(MeasureText(text.c_str(), fontSize) <= maxWidth) return text;
    string t = text;
    while(t.size()>0 && MeasureText((t+"...").c_str(), fontSize)>maxWidth)
        t.pop_back();
    return t+"...";
}

struct InputBox{
    string text;
    bool active = false;
    float cursorBlink = 0;

    void update(){
        if(!active) return;
        cursorBlink += GetFrameTime();
        int key = GetCharPressed();
        while(key>0){
            if(key>=32 && key<=125) text += (char)key;
            key = GetCharPressed();
        }
        if(IsKeyPressed(KEY_BACKSPACE) && !text.empty()) text.pop_back();
    }

    void draw(int x, int y, int w, int h, const char* placeholder, int fontSize=14){
        DrawRectangle(x,y,w,h,COL_BG);
        DrawRectangleLines(x,y,w,h, active ? COL_ACCENT : COL_BORDER);
        string display = text.empty() && !active ? placeholder : text;
        Color col = text.empty() ? COL_MUTED : COL_TEXT;
        string trunc = TruncateText(display, w-20, fontSize);
        DrawText(trunc.c_str(), x+10, y+(h-fontSize)/2, fontSize, col);
        if(active && (int)(cursorBlink*2)%2==0){
            int cx = x+10+MeasureText(TruncateText(text,w-20,fontSize).c_str(), fontSize);
            DrawRectangle(cx+2, y+8, 2, h-16, COL_ACCENT);
        }
    }

    string submit(){
        string val = text;
        text = "";
        active = false;
        return val;
    }
};

int main(){
    srand(time(0));

    InitWindow(480, 700, "MUSIC PLAYER - DSA MINI PROJECT");
    SetTargetFPS(60);

    MusicPlayer player;  //MusicPlayer from musicplayer.h
    InputBox addInput;
    int scrollOffset = 0;
    float spinAngle = 0;
    float progressAnim = 0;

    while(!WindowShouldClose()){

      
        float dt = GetFrameTime();
        addInput.update();
        

        if(!addInput.active){
            if(IsKeyPressed(KEY_LEFT)){
                player.prev();
                progressAnim = 0;
            }
            if(IsKeyPressed(KEY_RIGHT)){
                player.next();
                progressAnim = 0;
            }
        }

        if(player.isPlaying){
            spinAngle += dt * 120;
            progressAnim += dt / 30.0f;
            if(progressAnim > 1.0f) progressAnim = 0;
        }

        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !IsMouseOver(PX, 530, PW-90, 40))
            addInput.active = false;

        //Draw
        BeginDrawing();
        ClearBackground(COL_BG);
        int y = 0;

        //Header
        DrawRectangle(PX, 15, PW, 50, COL_ACCENT);
        DrawText("SOUNDLIST", PX+16, 28, 26, COL_BLACK);
        const char* tag = "DSA PROJECT";
        int tagW = MeasureText(tag, 11);
        DrawRectangle(PX+PW-tagW-24, 26, tagW+16, 22, COL_BLACK);
        DrawText(tag, PX+PW-tagW-16, 31, 11, COL_ACCENT);
        y = 65;

        //Now Playing
        DrawPanel(PX, y, PW, 90, COL_SURFACE, COL_BORDER);
        DrawText("NOW PLAYING", PX+14, y+10, 10, COL_MUTED);

        // Vinyl
        int vx=PX+20, vy=y+30, vr=22;
        DrawCircle(vx+vr, vy+vr, vr, player.isPlaying ? COL_ACCENT : COL_BORDER);
        for(int r=vr-2; r>4; r-=4){
            Color groove = player.isPlaying ?
                Color{(unsigned char)(200-r*3),(unsigned char)(245-r*2),66,255} :
                Color{30,30,30,255};
            DrawCircleLines(vx+vr, vy+vr, r, groove);
        }
        DrawCircle(vx+vr, vy+vr, 5, COL_BG);
        if(player.isPlaying){
            float rad = spinAngle * DEG2RAD;
            DrawLine(vx+vr, vy+vr,
                     vx+vr+(int)(cos(rad)*(vr-3)),
                     vy+vr+(int)(sin(rad)*(vr-3)),
                     COL_BLACK);
        }

        string nowSong = player.current ? player.current->song : "No song selected";
        Color nowCol = player.current ? COL_TEXT : COL_MUTED;
        DrawText(TruncateText(nowSong, PW-100, 16).c_str(), PX+70, y+28, 16, nowCol);

        if(player.isPlaying){
            DrawRectangle(PX+70, y+52, 60, 18, COL_ACCENT);
            DrawText("PLAYING", PX+76, y+55, 10, COL_BLACK);
        } else {
            DrawRectangle(PX+70, y+52, 48, 18, COL_BORDER);
            DrawText("PAUSED", PX+76, y+55, 10, COL_MUTED);
        }
        y += 90;

        //Progress Bar
        DrawRectangle(PX, y, PW, 6, COL_BORDER);
        if(player.isPlaying){
            DrawRectangle(PX, y, (int)(PW*progressAnim), 6, COL_ACCENT);
            DrawRectangle(PX+(int)(PW*progressAnim)-4, y-1, 8, 8, COL_ACCENT);
        }
        y += 6;

        //Controls
        DrawPanel(PX, y, PW, 64, COL_SURFACE, COL_BORDER);
        int btnY=y+12, btnH=38, bx=PX+10;

        if(DrawButton(bx, btnY, 80, btnH, "PLAY", COL_ACCENT, COL_BLACK, 13))
            if(!player.isPlaying) player.play();
        bx += 88;

        if(DrawButton(bx, btnY, 80, btnH, "STOP", COL_SURFACE, COL_ACCENT2, 13)){
            player.stopMusic();
            progressAnim = 0;
        }
        bx += 88;

        if(DrawButton(bx, btnY, 60, btnH, "< PREV", COL_SURFACE, COL_TEXT, 12)){
            player.prev();
            progressAnim = 0;
        }
        bx += 68;

        if(DrawButton(bx, btnY, 60, btnH, "NEXT >", COL_SURFACE, COL_TEXT, 12)){
            player.next();
            progressAnim = 0;
        }
        bx += 68;

        if(DrawButton(bx, btnY, 44, btnH, "RNG", COL_SURFACE, COL_YELLOW, 12)){
            player.randomSong();
            progressAnim = 0;
        }
        y += 64;

       // Playlist header
        DrawPanel(PX, y, PW, 30, COL_CARD, COL_BORDER);
        DrawText("PLAYLIST // CIRCULAR DOUBLY LINKED LIST", PX+12, y+9, 10, COL_MUTED);
        string countStr = "[" + to_string(player.size) + " SONGS]";
        int cw = MeasureText(countStr.c_str(), 10);
        DrawText(countStr.c_str(), PX+PW-cw-12, y+9, 10, COL_ACCENT);
        y += 30;

        //Playlist items
        auto songs = player.getSongs();
        int rowH = 44;
        int playlistH = PLAYLIST_ROWS * rowH;

        DrawRectangle(PX, y, PW, playlistH, COL_CARD);
        DrawRectangleLines(PX, y, PW, playlistH, COL_BORDER);

        if(songs.empty()){
            const char* emptyMsg = "// PLAYLIST IS EMPTY - ADD A SONG BELOW";
            int ew = MeasureText(emptyMsg, 12);
            DrawText(emptyMsg, PX+(PW-ew)/2, y+playlistH/2-6, 12, COL_MUTED);
        } else {
            float wheel = GetMouseWheelMove();
            if(wheel!=0 && IsMouseOver(PX,y,PW,playlistH)){
                scrollOffset -= (int)wheel;
                scrollOffset = max(0, min(scrollOffset, (int)songs.size()-PLAYLIST_ROWS));
            }

            int currentIdx = 0;
            for(int i=0; i<(int)songs.size(); i++)
                if(songs[i].second){ currentIdx=i; break; }
            if(currentIdx < scrollOffset) scrollOffset = currentIdx;
            if(currentIdx >= scrollOffset+PLAYLIST_ROWS) scrollOffset = currentIdx-PLAYLIST_ROWS+1;
            scrollOffset = max(0, scrollOffset);

            for(int i=scrollOffset; i<min((int)songs.size(), scrollOffset+PLAYLIST_ROWS); i++){
                int iy = y+(i-scrollOffset)*rowH;
                bool isCurrent = songs[i].second;

                if(isCurrent){
                    DrawRectangle(PX+1, iy, PW-2, rowH, COL_ACTIVE_BG);
                    DrawRectangle(PX+1, iy, 3, rowH, COL_ACCENT);
                }
                if(!isCurrent && IsMouseOver(PX,iy,PW,rowH)){
                    DrawRectangle(PX+1, iy, PW-2, rowH, Color{255,255,255,8});
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                }

                DrawLine(PX, iy+rowH, PX+PW, iy+rowH, COL_BORDER);

                string numStr = to_string(i+1);
                if(numStr.size()==1) numStr = "0"+numStr;
                DrawText(numStr.c_str(), PX+14, iy+(rowH-14)/2, 13,
                         isCurrent ? COL_ACCENT : COL_MUTED);

                string songName = TruncateText(songs[i].first, PW-100, 14);
                DrawText(songName.c_str(), PX+46, iy+(rowH-14)/2, 14,
                         isCurrent ? COL_ACCENT : COL_TEXT);

                // Animated bars when playing
                if(isCurrent && player.isPlaying){
                    int bx2=PX+PW-54, barW=5, barGap=3;
                    for(int b=0; b<4; b++){
                        float phase=(float)b/4.0f;
                        float h2=(sin(GetTime()*5+phase*6.28f)*0.5f+0.5f)*18+4;
                        DrawRectangle(bx2+b*(barW+barGap), iy+rowH/2-(int)h2/2, barW, (int)h2, COL_ACCENT);
                    }
                }

                // Delete button
                int dx=PX+PW-26, dy2=iy+(rowH-22)/2;
                if(IsMouseOver(dx,dy2,22,22)) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                if(DrawButton(dx,dy2,22,22,"x",COL_SURFACE,COL_MUTED,12)){
                    if(!player.isPlaying) player.deleteMusic(i+1);
                    else player.logMsg = "STOP PLAYBACK BEFORE DELETING";
                }

                // Click row to select and play
                if(IsClicked(PX, iy, PW-30, rowH)){
                    bool wasPlaying = player.isPlaying;
                    if(wasPlaying) player.stopMusic();
                    player.setByIndex(i+1);
                    progressAnim = 0;
                    if(wasPlaying) player.play();
                }
            }

            // Scrollbar drawing
if((int)songs.size() > PLAYLIST_ROWS){

    float scrollSpeed = 14.0f;

    smoothScroll += (scrollOffset - smoothScroll) * scrollSpeed * GetFrameTime();

    int maxScroll = (int)songs.size() - PLAYLIST_ROWS;

    float pos = (maxScroll > 0) ? (smoothScroll / maxScroll) : 0;

    int sbH = max(20, (int)(playlistH * ((float)PLAYLIST_ROWS / songs.size())));
    int sbY = y + (int)((playlistH - sbH) * pos);

    DrawRectangle(PX+PW-5, y, 4, playlistH, COL_BORDER);
    DrawRectangle(PX+PW-5, sbY, 4, sbH, COL_ACCENT);
}
        }
        y += playlistH;

        //Add Song Input
        DrawPanel(PX, y, PW, 60, COL_SURFACE, COL_BORDER);
        if(IsClicked(PX+10, y+10, PW-100, 40)) addInput.active = true;
        addInput.draw(PX+10, y+10, PW-100, 40, "ENTER SONG NAME...", 13);
        if(DrawButton(PX+PW-84, y+10, 74, 40, "+ ADD", COL_ACCENT, COL_BLACK, 13) ||
           (addInput.active && IsKeyPressed(KEY_ENTER))){
            string val = addInput.submit();
            if(!val.empty()) player.addMusic(val);
        }
        y += 60;

        
        DrawPanel(PX, y, PW, 32, Color{13,13,13,255}, COL_BORDER);
        float pulse = sin(GetTime()*3)*0.5f+0.5f;
        Color dotCol = player.isPlaying ?
            Color{200,245,66,(unsigned char)(150+(int)(pulse*105))} : COL_MUTED;
        DrawCircle(PX+18, y+16, 5, dotCol);
        DrawText(TruncateText(player.logMsg, PW-50, 11).c_str(), PX+30, y+10, 11, COL_MUTED);
        y += 32;

        //Footer
        DrawPanel(PX, y, PW, 26, COL_CARD, COL_BORDER);
        DrawText("struct node { song | *next | *prev }", PX+12, y+7, 10, COL_MUTED);
        const char* dsa = "HEAD -> TAIL -> HEAD";
        DrawText(dsa, PX+PW-MeasureText(dsa,10)-12, y+7, 10, COL_ACCENT);

        EndDrawing();
    }

    player.stopMusic();
    CloseWindow();
    return 0;
}