#include "minorGems/graphics/openGL/JoyHandlerGL.h"
#include "minorGems/graphics/openGL/KeyboardHandlerGL.h"
#include "minorGems/graphics/openGL/ScreenGL.h"
#include "hetuwmod.h"
#include "LivingLifePage.h"
#include "SDL/SDL.h"
#include "math.h"

static int joyPadDir = 0; // check Dpad values in joyHandlerGL.h

static bool joyAButton = false;
static bool joyBButton = false;
static bool joyXButton = false;
static bool joyYButton = false;

// buttons near the rudders
static bool joyLButton = false;
static bool joyRButton = false;

// when you press the thumbstick
static bool joyLThumbButton = false;
static bool joyRThumbButton = false;

static bool joyStartButton = false;
static bool joySelectButton = false;

// we handle rudders as buttons, (like ctrl or alt)
static bool joyLRudder = false;
static bool joyRRudder = false;

// left and right thumbsticks directions as x, y values they generally range in -32,768 to 32,767, and 0 when not active
static int joyLThumbX = 0;
static int joyLThumbY = 0;
static int joyRThumbX = 0;
static int joyRThumbY = 0;

static int padMap[10] = {0};
static int rudderMap[10] = {0};
static int thumbstickMap[10] = {0};
static int buttonMap[10] = {0};


// return true if event was consumed
bool mapJoyThumbstick(int &stick, int &x, int &y)
{
    if (abs(x) < JOY_DEADZONE)
        x = 0;
    if (abs(y) < JOY_DEADZONE)
        y = 0;

    // my controller has some axis flipped (or at least what SDL thinks..)
    if (stick == JOY_L_THUMB && y != 0)
        y *= -1;
    if(stick == JOY_R_THUMB) 
    {
        int tx = x;
        x = y;
        y = -tx;
    }
    return false;
}

bool mapJoyPad(int &dir)
{
    return false;
}

bool mapJoyButton(int &button)
{
    // my controller buttons are swapped
    if (button == JOY_B)
        button = JOY_A;
    else if (button == JOY_Y)
        button = JOY_X;
    else if (button == JOY_X)
        button = JOY_Y;
    else if (button == JOY_A)
        button = JOY_B;
    return false;
}

bool mapJoyRudder(int &rudder, int &pressure)
{
    return false;
}

void LivingLifePage::joyDPadDown(int dir)
{
    if (mapJoyPad(dir))
        return;
    printf(">>> LivingLifePage::joyDPadDown dir=%d joyRRudder=%d joyRButton=%d joyLRudder=%d joyLButton=%d\n",
           dir, joyRRudder, joyRButton, joyLRudder, joyLButton);
    joyPadDir = dir;

    if (joyPadDir != 0)
    {
        LiveObject *ourLiveObject = getOurLiveObject();
        if (!ourLiveObject)
            return;
        bool isFaceDirectionChange = false;
        bool isBabyAction = false;
        bool isAlphaAction = false;
        bool isBetaAction = false;
        bool isZoomControllAction = false;
        bool isSingleMoveAction = false;

        SDLMod oldModState = SDL_GetModState();
        if (!joyRRudder && !joyRButton && !joyLRudder && !joyLButton) // move action
        {
            if (joyPadDir == JOY_E)
            {
                HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Right);
                HetuwMod::move();
                HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Right);
            }
            else if (joyPadDir == JOY_W)
            {
                HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Left);
                HetuwMod::move();
                HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Left);
            }
            else if (joyPadDir == JOY_N)
            {
                HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Up);
                HetuwMod::move();
                HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Up);
            }
            else if (joyPadDir == JOY_S)
            {
                HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Down);
                HetuwMod::move();
                HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Down);
            }
        }
        else if (joyRRudder && !joyRButton && !joyLRudder && !joyLButton)
        {
            if (joyPadDir == JOY_E)
            {
                HetuwMod::actionBetaRelativeToMe(1, 0);
            }
            else if (joyPadDir == JOY_W)
            {
                HetuwMod::actionBetaRelativeToMe(-1, 0);
            }
            else if (joyPadDir == JOY_N)
            {
                HetuwMod::actionBetaRelativeToMe(0, 1);
            }
            else if (joyPadDir == JOY_S)
            {
                HetuwMod::actionBetaRelativeToMe(0, -1);
            }
        }
        else if (!joyRRudder && joyRButton && !joyLRudder && !joyLButton)
        {
            if (joyPadDir == JOY_E)
            {
                HetuwMod::actionAlphaRelativeToMe(1, 0);
            }
            else if (joyPadDir == JOY_W)
            {
                HetuwMod::actionAlphaRelativeToMe(-1, 0);
            }
            else if (joyPadDir == JOY_N)
            {
                HetuwMod::actionAlphaRelativeToMe(0, 1);
            }
            else if (joyPadDir == JOY_S)
            {
                HetuwMod::actionAlphaRelativeToMe(0, -1);
            }
        }
        else if (!joyRRudder && !joyRButton && !joyLRudder && joyLButton) // change face direction action
        {
            if (joyPadDir == JOY_E && !ourLiveObject->inMotion)
            {
                // change face to look east
                pointerMove(ourLiveObject->currentPos.x * CELL_D + CELL_D, ourLiveObject->currentPos.y * CELL_D);
            }
            else if (joyPadDir == JOY_W && !ourLiveObject->inMotion)
            {
                // change face to look west
                pointerMove(ourLiveObject->currentPos.x * CELL_D - CELL_D, ourLiveObject->currentPos.y * CELL_D);
            }
        }
        else if (joyRRudder && !joyRButton && joyLRudder && joyLButton)
        {
            isZoomControllAction = true;
        }
        else if (!joyRRudder && joyRButton && !joyLRudder && joyLButton)
        {
            if (ourLiveObject->holdingID < 0)
            {
                // we are already holding a baby
                // translate it into a drop action
                isBetaAction = true;
            }
            else if (ourLiveObject->holdingID == 0)
            {
                isBabyAction = true;
            }
        }
    }
}

void LivingLifePage::joyDPadUp()
{
    printf(">>> LivingLifePage::joyDPadUp\n");
    joyPadDir = 0;
}

void LivingLifePage::joyButtonDown(int button)
{
    if (mapJoyButton(button))
        return;
    printf(">>> LivingLifePage::joyButtonDown button=%d\n", button);
    if (button == JOY_A)
        joyAButton = true;
    else if (button == JOY_B)
        joyBButton = true;
    else if (button == JOY_X)
        joyXButton = true;
    else if (button == JOY_Y)
        joyYButton = true;
    else if (button == JOY_R)
        joyRButton = true;
    else if (button == JOY_L)
        joyLButton = true;
    else if (button == JOY_AXIS_R)
        joyRThumbButton = true;
    else if (button == JOY_AXIS_L)
        joyLThumbButton = true;
    else if (button == JOY_SELECT)
        joySelectButton = true;
    else if (button == JOY_START)
        joyStartButton = true;
    else
        printf(">>> warning: unkown button %d in "
               "LivingLifePage::joyButtonDown\n",
               button);
    if (button == JOY_Y)
    {
        // HetuwMod::useOnSelf();
        HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Eat);
        HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Eat);
    }
    else if (button == JOY_B)
    {
        if (!joyRRudder && !joyRButton && !joyLRudder && joyLButton)
        {
            HetuwMod::livingLifeKeyDown(HetuwMod::charKey_TakeOffBackpack);
            HetuwMod::livingLifeKeyUp(HetuwMod::charKey_TakeOffBackpack);
        }
        else
        {
            SDLMod oldModState = SDL_GetModState();
            if (!joyRRudder && !joyRButton && !joyLRudder && !joyLButton)
                SDL_SetModState(KMOD_LCTRL);
            else if (!joyRRudder && joyRButton && !joyLRudder && !joyLButton)
                SDL_SetModState(KMOD_LSHIFT);
            HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Backpack);
            HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Backpack);
            SDL_SetModState(oldModState);
        }

    }
    else if (button == JOY_A)
    {
        if (joyRRudder && !joyRButton && !joyLRudder && !joyLButton)
        {
            HetuwMod::actionBetaRelativeToMe(0, 0);
        } else if (!joyRRudder && joyRButton && !joyLRudder && !joyLButton)
        {
            HetuwMod::actionAlphaRelativeToMe(0, 0);
        }
    }
    else if (button == JOY_X)
    {
        if (joyLButton)
        {
            if (!HetuwMod::bDrawYum)
                HetuwMod::livingLifeKeyDown(HetuwMod::charKey_FindYum);
        }
    }
    // needs SDL2 or higher, wanted to test it and implement it along with starving sound, when the starve sound hits the joystick rumbbles
    // int accetped = SDL_JoystickRumbleTriggers(getJoystick(), 0xFFFF, 0xFFFF, 500);
}

void LivingLifePage::joyButtonUp(int button)
{
    if (mapJoyButton(button))
        return;
    printf(">>> LivingLifePage::joyButtonUp button=%d\n", button);
    if (button == JOY_A)
        joyAButton = false;
    else if (button == JOY_B)
        joyBButton = false;
    else if (button == JOY_X)
        joyXButton = false;
    else if (button == JOY_Y)
        joyYButton = false;
    else if (button == JOY_R)
        joyRButton = false;
    else if (button == JOY_L)
        joyLButton = false;
    else if (button == JOY_AXIS_R)
        joyRThumbButton = false;
    else if (button == JOY_AXIS_L)
        joyLThumbButton = false;
    else if (button == JOY_SELECT)
        joySelectButton = false;
    else if (button == JOY_START)
        joyStartButton = false;
    else
        printf(">>> warning: unkown button %d in "
               "LivingLifePage::joyButtonDown\n",
               button);

    if (button == JOY_X || button == JOY_L)
    {
        if (HetuwMod::bDrawYum)
            HetuwMod::livingLifeKeyUp(HetuwMod::charKey_FindYum);
    }
}

static bool sawNegativePressure = false;
void LivingLifePage::joyRudder(int rudder, int pressure)
{
    if (mapJoyRudder(rudder, pressure))
        return;
    printf(">>> LivingLifePage::joyRudder rudder=%d pressure=%d\n", rudder, pressure);
    // we will implement the rudders as buttons (used like alt or ctrl key)
    if (rudder == JOY_R_RUDDER)
        joyRRudder = pressure > -1;
    if (rudder == JOY_L_RUDDER)
    {
        // weird stuff!
        // same rudder(left) when i press both right and left, but pressure comes +32640 when i
        // press leftRudder and -32640 when i press rightRudder, idk if this is only for me
        if (pressure < -1)
        { 
            joyRRudder = true;
        }
        else if(pressure > -1)
        {
            joyLRudder = true;
        } 
        else 
        {
            joyLRudder = false;
            joyRRudder = false;
        }
    }
}

void LivingLifePage::joyThumbstick(int stick, int sx, int sy)
{
    printf(">>> LivingLifePage::joyThumbstickBeforeMap "
           "stick=%d x=%d y=%d\n",
           stick, sx, sy);
    if (mapJoyThumbstick(stick, sx, sy))
        return;
    printf(">>> LivingLifePage::joyThumbstick "
           "stick=%d x=%d y=%d\n",
           stick, sx, sy);
    if (stick == JOY_L_THUMB)
    {
        joyLThumbX = sx;
        joyLThumbY = sy;
    }
    else if (stick == JOY_R_THUMB)
    {
        joyRThumbX = sx;
        joyRThumbY = sy;
    }
    else
    {
        printf(">>> warning: unknown stick number %d "
               "in LivingLifePage::joyThumbstick\n",
               stick);
    }

    if (joyLThumbY == 0)
    {
        printf(">>> LivingLifePage::joyThumbstickStopMove "
           "stick=%d x=%d y=%d\n",
           stick, sx, sy);
        if (HetuwMod::upKeyDown)
            HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Up);
        if (HetuwMod::downKeyDown)
            HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Down);
    }
    else if (joyLThumbY > 0)
    {
        printf(">>> LivingLifePage::joyThumbstickMoveUp "
           "stick=%d x=%d y=%d\n",
           stick, sx, sy);
        if(!HetuwMod::upKeyDown)
            HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Up);
    }
    else if (joyLThumbY < 0)
    {
        printf(">>> LivingLifePage::joyThumbstickMoveDown "
           "stick=%d x=%d y=%d\n",
           stick, sx, sy);
        if(!HetuwMod::downKeyDown)
            HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Down);
    }

    if (joyLThumbX == 0)
    {
        if (HetuwMod::leftKeyDown)
            HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Left);
        if (HetuwMod::rightKeyDown)
            HetuwMod::livingLifeKeyUp(HetuwMod::charKey_Right);
    }
    else if (joyLThumbX > 0)
    {
        if(!HetuwMod::rightKeyDown)
            HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Right);
    }
    else if (joyLThumbX < 0)
    {
        if(!HetuwMod::leftKeyDown)
            HetuwMod::livingLifeKeyDown(HetuwMod::charKey_Left);
    }
}

void LivingLifePage::joyStep(void)
{
    LiveObject *ourLiveObject = getOurLiveObject();
    if (!ourLiveObject)
        return;
    double currentTime = game_getCurrentTime();
}
