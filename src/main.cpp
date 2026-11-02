#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cmath>
#include <string>

struct Building {
    Vector3 pos;
    Vector3 size;
    Color color;
    bool shop;
};

struct Palm {
    Vector3 pos;
    float scale;
};

struct Person {
    Vector3 pos;
    float phase;
};

static float ClampF(float x, float a, float b) {
    return x < a ? a : (x > b ? b : x);
}

static void DrawPalm(Vector3 p, float s) {
    Vector3 trunkTop = {p.x, p.y + 5.0f*s, p.z};
    DrawCylinderEx(p, trunkTop, 0.25f*s, 0.16f*s, 8, BROWN);
    for (int i = 0; i < 9; ++i) {
        float a = i * 2.0f * PI / 9.0f;
        Vector3 end = {p.x + cosf(a)*3.0f*s, p.y + 5.9f*s, p.z + sinf(a)*3.0f*s};
        Vector3 mid = {p.x + cosf(a)*1.4f*s, p.y + 6.1f*s, p.z + sinf(a)*1.4f*s};
        DrawCylinderEx(trunkTop, mid, 0.16f*s, 0.10f*s, 5, DARKGREEN);
        DrawCylinderEx(mid, end, 0.10f*s, 0.015f*s, 5, (i%2)?LIME:GREEN);
    }
    DrawSphere({p.x,p.y+5.9f*s,p.z},0.3f*s, DARKGREEN);
}

static void DrawBuilding(const Building& b) {
    Vector3 center = {b.pos.x, b.size.y/2.0f, b.pos.z};
    DrawCube(center, b.size.x, b.size.y, b.size.z, b.color);
    DrawCubeWires(center, b.size.x, b.size.y, b.size.z, Fade(BLACK,0.18f));
    // Roof parapet and water tank
    DrawCube({b.pos.x,b.size.y+0.18f,b.pos.z},b.size.x+0.25f,0.35f,b.size.z+0.25f,BEIGE);
    DrawCylinder({b.pos.x+b.size.x*.22f,b.size.y+0.9f,b.pos.z-b.size.z*.2f},
                 0.48f,0.48f,0.9f,12,GRAY);
    // Front windows
    int floors = (int)(b.size.y/3.0f);
    for(int f=0; f<floors; ++f) {
        for(float x=-b.size.x*.32f; x<=b.size.x*.32f; x+=b.size.x*.34f) {
            Vector3 w={b.pos.x+x,1.8f+f*3.0f,b.pos.z+b.size.z/2.0f+0.035f};
            DrawCube(w,0.95f,1.15f,0.08f,(Color){68,119,139,255});
            DrawCubeWires(w,1.0f,1.2f,0.1f,DARKGRAY);
        }
    }
    if(b.shop) {
        DrawCube({b.pos.x,2.55f,b.pos.z+b.size.z/2.0f+0.55f},
                 b.size.x+0.7f,0.32f,1.1f,ORANGE);
        DrawCube({b.pos.x,3.2f,b.pos.z+b.size.z/2.0f+0.1f},
                 b.size.x*.72f,0.5f,0.12f,DARKBLUE);
    }
}

static void DrawCar(Vector3 p, float yaw) {
    rlPushMatrix();
    rlTranslatef(p.x,p.y,p.z);
    rlRotatef(yaw*RAD2DEG,0,1,0);
    DrawCube({0,1.0f,0},4.1f,1.1f,6.7f,MAROON);
    DrawCube({0,2.05f,-0.35f},3.35f,1.15f,3.3f,(Color){44,68,80,255});
    DrawCube({0,2.66f,-0.35f},3.4f,0.15f,3.35f,RED);
    for(float x : {-2.05f,2.05f}) {
        for(float z : {-2.15f,2.15f}) {
            DrawCylinderEx({x,0.68f,z},{x+(x>0?0.01f:-0.01f),0.68f,z},
                           0.68f,0.68f,16,BLACK);
            DrawCylinder({x,0.68f,z},0.28f,0.28f,0.45f,12,LIGHTGRAY);
        }
    }
    DrawCube({-1.25f,1.25f,3.38f},0.65f,0.28f,0.08f,YELLOW);
    DrawCube({1.25f,1.25f,3.38f},0.65f,0.28f,0.08f,YELLOW);
    DrawCube({0,1.3f,-3.38f},3.1f,0.22f,0.08f,RED);
    rlPopMatrix();
}

int main() {
    const int screenWidth=1280, screenHeight=720;
    InitWindow(screenWidth,screenHeight,"Qeshm Life - C++ Open World");
    SetTargetFPS(60);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    Camera3D camera = {};
    camera.position={0,7,15};
    camera.target={0,1,0};
    camera.up={0,1,0};
    camera.fovy=60;
    camera.projection=CAMERA_PERSPECTIVE;

    std::vector<Building> buildings;
    Color palette[] = {
        (Color){231,222,201,255}, (Color){197,181,151,255},
        (Color){30,128,137,255}, (Color){205,119,83,255},
        (Color){67,125,151,255}, (Color){220,190,132,255}
    };

    // Compact coastal town layout: blocks around a central street grid.
    for(int row=0;row<4;++row) {
        for(int col=0;col<7;++col) {
            float x=-92+col*29+(row%2)*5;
            float z=-65+row*31;
            if (fabsf(z-10)<12 || fabsf(x+45)<8 || fabsf(x-55)<8 ||
                fabsf(z+48)<8 || fabsf(z-57)<8) continue;
            Building b;
            b.pos={x,0,z};
            b.size={13.0f+(col%3)*2.0f,7.0f+(row%3)*3.0f,12.0f+(row%2)*3.0f};
            b.color=palette[(row*7+col)%6];
            b.shop=((row==1 || row==2) && col%2==0);
            buildings.push_back(b);
        }
    }
    for(int i=0;i<8;++i) {
        Building b;
        b.pos={-84+i*24.0f,0,30};
        b.size={17,5.5f,12};
        b.color=palette[i%6]; b.shop=true;
        buildings.push_back(b);
    }
    buildings.push_back({{-12,0,-29},{25,11,17},palette[2],true});
    buildings.push_back({{88,0,-42},{20,8,15},palette[5],true});
    buildings.push_back({{-112,0,-38},{18,7,15},palette[3],true});

    std::vector<Palm> palms;
    for(int i=0;i<44;++i) {
        float x=-125+(i%11)*24.0f;
        float z=-75+(i/11)*45.0f;
        if(fabsf(z-10)<13 || fabsf(x+45)<8 || fabsf(x-55)<8) continue;
        palms.push_back({{x,0,z},0.8f+(i%4)*0.12f});
    }
    for(int i=0;i<14;++i) palms.push_back({{-120+i*17.0f,0,73},0.9f});

    std::vector<Person> people;
    for(int i=0;i<12;++i)
        people.push_back({{-90+(i%6)*32.0f,0,-5+(i/6)*34.0f},i*.8f});

    Vector3 player={0,0,35};
    float playerYaw=PI, carYaw=PI;
    Vector3 car={12,0,18};
    bool driving=false, running=false, night=false, showMap=true;
    float camYaw=0.0f, camPitch=0.18f;
    Vector2 lastMouse=GetMousePosition();
    bool dragging=false;
    float toastTime=0;
    std::string toast="Welcome to Qeshm Island!";
    float elapsed=0;

    while(!WindowShouldClose()) {
        float dt=GetFrameTime();
        elapsed+=dt;
        if(IsKeyPressed(KEY_E)) {
            if(!driving && Vector3Distance(player,car)<9.0f) {
                driving=true; toast="You entered the car"; toastTime=2.0f;
            } else if(driving) {
                driving=false; player=Vector3Add(car,{5,0,0});
                toast="You left the car"; toastTime=2.0f;
            } else {toast="Get closer to the car";toastTime=2.0f;}
        }
        if(IsKeyPressed(KEY_LEFT_SHIFT)) running=!running;
        if(IsKeyPressed(KEY_N)) night=!night;
        if(IsKeyPressed(KEY_M)) showMap=!showMap;

        // Mouse camera orbit
        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {dragging=true;lastMouse=GetMousePosition();}
        if(IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) dragging=false;
        if(dragging) {
            Vector2 now=GetMousePosition();
            Vector2 delta=Vector2Subtract(now,lastMouse);
            camYaw-=delta.x*0.004f;
            camPitch=ClampF(camPitch+delta.y*0.0025f,-0.05f,0.65f);
            lastMouse=now;
        }

        float forward=(IsKeyDown(KEY_W)||IsKeyDown(KEY_UP)?1.0f:0.0f)-
                     (IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN)?1.0f:0.0f);
        float side=(IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT)?1.0f:0.0f)-
                   (IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT)?1.0f:0.0f);
        Vector3 dir={sinf(camYaw)*forward+cosf(camYaw)*side,0,
                     cosf(camYaw)*forward-sinf(camYaw)*side};
        if(Vector3Length(dir)>1) dir=Vector3Normalize(dir);

        if(driving) {
            if(Vector3Length(dir)>0.01f) {
                carYaw=atan2f(dir.x,dir.z);
                car=Vector3Add(car,Vector3Scale(dir,16.0f*dt));
            }
            car.x=ClampF(car.x,-135,135); car.z=ClampF(car.z,-92,95);
            player=car;
        } else {
            float speed=running?9.0f:5.0f;
            if(Vector3Length(dir)>0.01f) {
                player=Vector3Add(player,Vector3Scale(dir,speed*dt));
                playerYaw=atan2f(dir.x,dir.z);
            }
            player.x=ClampF(player.x,-135,135);player.z=ClampF(player.z,-92,95);
        }

        Vector3 focus=driving?car:player;
        float distance=driving?15.0f:10.0f;
        camera.position={focus.x+sinf(camYaw)*distance,
                         focus.y+3.6f+camPitch*7.0f,
                         focus.z+cosf(camYaw)*distance};
        camera.target={focus.x,focus.y+1.5f,focus.z};

        Color sky=night?(Color){17,29,54,255}:(Color){139,198,218,255};
        Color ambient=night?(Color){45,58,82,255}:(Color){230,222,199,255};
        BeginDrawing();
        ClearBackground(sky);
        BeginMode3D(camera);

        // island ground and sea
        DrawCube({0,-0.55f,0},300,0.8f,210,(Color){218,190,143,255});
        DrawPlane({0,-0.48f,-155},{900,900},(Color){20,137,159,255});
        DrawPlane({0,-0.12f,0},{440,340},(Color){215,190,145,255});

        // rocky inland ridge
        for(int i=0;i<24;++i) {
            float x=-140+i*12.0f;
            float z=-92+sinf(i*.7f)*7;
            DrawCylinder({x,7,z},5.5f,9.0f,18.0f,7,(Color){130,111,87,255});
        }

        // roads
        DrawCube({0,0.02f,10},245,0.13f,10,(Color){52,59,63,255});
        DrawCube({-45,0.02f,-25},10,0.13f,115,(Color){52,59,63,255});
        DrawCube({55,0.02f,-25},10,0.13f,115,(Color){52,59,63,255});
        DrawCube({5,0.02f,-48},130,0.13f,9,(Color){52,59,63,255});
        DrawCube({5,0.02f,57},130,0.13f,9,(Color){52,59,63,255});
        DrawCube({-105,0.02f,25},9,0.13f,75,(Color){52,59,63,255});
        DrawCube({105,0.02f,20},9,0.13f,90,(Color){52,59,63,255});
        DrawCube({0,0.02f,-83},220,0.13f,8,(Color){52,59,63,255});
        for(int x=-118;x<120;x+=10) DrawCube({(float)x,0.1f,10},3,0.04f,.16f,BEIGE);
        for(int x=-118;x<120;x+=10) DrawCube({(float)x,0.1f,-83},3,0.04f,.16f,BEIGE);

        for(const auto& b:buildings) DrawBuilding(b);
        for(const auto& p:palms) DrawPalm(p.pos,p.scale);

        // waterfront promenade
        for(int i=0;i<22;++i) {
            float x=-125+i*12.0f;
            DrawCylinder({x,2.5f,83},.12f,.09f,5.0f,8,DARKGRAY);
            DrawCube({x+.55f,4.8f,83},1.2f,.1f,.12f,DARKGRAY);
            DrawSphere({x+1.05f,4.65f,83},.2f,YELLOW);
        }
        // benches
        for(int i=0;i<8;++i) {
            float x=-95+i*26.0f;
            DrawCube({x,.72f,68},3,.18f,.8f,BROWN);
            DrawCube({x-1,.35f,68},.15f,.7f,.15f,DARKBROWN);
            DrawCube({x+1,.35f,68},.15f,.7f,.15f,DARKBROWN);
        }
        // harbor boats
        for(int i=0;i<5;++i) {
            float x=-85+i*35.0f,z=104+(i%2)*5.0f;
            DrawCube({x,.35f,z},3.2f,.65f,7.0f,(i%2)?WHITE:RED);
            DrawCube({x,1.0f,z-.5f},2.0f,1.0f,2.2f,BEIGE);
        }

        // small mosque landmark
        DrawCube({88,4,42},20,8,16,WHITE);
        DrawSphere({88,8,42},4.8f,(Color){75,153,150,255});
        for(float x:{81.0f,95.0f}) {
            DrawCylinder({x,8.5f,37},.9f,.75f,17,12,WHITE);
            DrawCylinder({x,18.2f,37},0,1.0f,2.7f,12,(Color){35,125,125,255});
        }

        // ambient town pedestrians
        for(const auto& p:people) {
            float px=p.pos.x+sinf(elapsed*.6f+p.phase)*1.1f;
            DrawCylinder({px,1.05f,p.pos.z},.28f,.34f,1.5f,(Color){75,113,126,255});
            DrawSphere({px,2.0f,p.pos.z},.25f,(Color){185,125,88,255});
        }

        DrawCar(car,carYaw);

        // player model
        if(!driving) {
            rlPushMatrix();
            rlTranslatef(player.x,player.y,player.z);
            rlRotatef(playerYaw*RAD2DEG,0,1,0);
            DrawCapsule({0,.9f,0},{0,2.15f,0},.42f,6,8,(Color){43,79,104,255});
            DrawSphere({0,2.65f,0},.36f,(Color){183,119,82,255});
            DrawSphere({0,2.82f,-.02f},.36f,DARKBROWN);
            DrawCapsule({-.24f,.45f,0},{-.24f,1.05f,0},.17f,4,6,(Color){47,56,68,255});
            DrawCapsule({.24f,.45f,0},{.24f,1.05f,0},.17f,4,6,(Color){47,56,68,255});
            rlPopMatrix();
        }
        EndMode3D();

        // HUD
        DrawRectangle(15,15,245,74,Fade(BLACK,.55f));
        DrawText("QESHM LIFE",28,22,24,WHITE);
        DrawText(driving?"DRIVING  |  E: EXIT":"ON FOOT  |  E: ENTER CAR",29,54,12, (Color){255,207,143,255});
        DrawRectangle(15,screenHeight-48,340,32,Fade(BLACK,.5f));
        DrawText("WASD: MOVE   SHIFT: RUN   RIGHT MOUSE: CAMERA",25,screenHeight-40,11,WHITE);
        DrawText("N: DAY/NIGHT   M: MAP",25,screenHeight-23,10,LIGHTGRAY);

        if(showMap) {
            int mx=screenWidth-190,my=20;
            DrawRectangle(mx,my,170,170,Fade((Color){8,35,43,255},.82f));
            DrawRectangle(mx+5,my+125,160,40,(Color){20,137,159,255});
            for(int x=mx+8;x<mx+165;x+=27)
                for(int y=my+10;y<my+120;y+=25)
                    DrawRectangle(x,y,13,9,(Color){196,174,137,255});
            DrawRectangle(mx+5,my+83,160,5,DARKGRAY);
            DrawRectangle(mx+54,my+5,5,120,DARKGRAY);
            DrawCircle(mx+85+(int)(player.x*.48f),my+82+(int)(player.z*.48f),5,YELLOW);
            DrawCircle(mx+85+(int)(car.x*.48f),my+82+(int)(car.z*.48f),4,RED);
            DrawText("MAP",mx+8,my+7,10,WHITE);
        }
        if(toastTime>0) {
            toastTime-=dt;
            int tw=MeasureText(toast.c_str(),18);
            DrawRectangle(screenWidth/2-tw/2-18,screenHeight-110,tw+36,38,Fade(BLACK,.65f));
            DrawText(toast.c_str(),screenWidth/2-tw/2,screenHeight-100,18,WHITE);
        }
        DrawFPS(screenWidth-85,screenHeight-28);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
