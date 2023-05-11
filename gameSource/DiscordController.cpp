// ############################################################################
// ############################################################################
// ############################################################################
// ############################################################################
// ############################################################################
// TODO: changing this file or it's .h does not recall the build of .o object !!
// ############################################################################
// ############################################################################
// ############################################################################
// ############################################################################
// ############################################################################

#include <cstdio>
#include <cassert>
#include <cstring>
#include <climits>
#include <ctime>

#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/system/Thread.h"

#include "objectBank.h"
#include "LivingLifePage.h"
#include "DiscordController.h"
#include "emotion.h"

#include <time.h>

extern char userReconnect;

char dDisplayDetails = true; // overriden from setttings
char dDisplayGame = true;    // overriden from setttings
time_t lastReconnectAttempt = 0;

char *dLastReportedName = NULL;
int dLastReportedAge = -1;
char dLastReportedFertility = false;
char dLastReportedAfk = false;
char dLastReportDetails = false; // overriden from setttings
 // did we report anything previously?
char dLastReport = false;

static int afkEmotionIndex = 21; // value is from server settings afkEmotionIndex.ini

void DISCORD_CALLBACK OnActivityUpdate(void *data, EDiscordResult result)
{
    printf("OnActivityUpdate %d was returned\n", (int)result);
    if (result != EDiscordResult::DiscordResult_Ok)
    {
        printf("discord error: activity update failed result %d was returned\n", (int)result);
    }
}

DiscordController::DiscordController()
{
    memset(&app, 0, sizeof(app));
    dDisplayGame = SettingsManager::getIntSetting("discordRichPresence", 0) > 0;
    dDisplayDetails = SettingsManager::getIntSetting("discordRichPresenceDetails", 0) > 0;
    printf("discord: discordRichPresence.ini=%d\n", dDisplayGame);
    printf("discord: discordRichPresenceDetails.ini=%d\n", dDisplayDetails);
    dLastReportDetails = dDisplayDetails;
    discordControllerInstance = this;
}

EDiscordResult DiscordController::connect()
{
    lastReconnectAttempt = time(0);
    if (!dDisplayGame)
    {
        printf("discord warning connect(): setting dDisplayGame is false(discord_rich_presence.ini=0), will not attempt to connect, we will return %d to our caller\n", EDiscordResult::DiscordResult_ApplicationMismatch);
        return EDiscordResult::DiscordResult_ApplicationMismatch;
    }
    struct IDiscordActivityEvents activities_events;
    memset(&activities_events, 0, sizeof(activities_events));

    struct DiscordCreateParams params;
    DiscordCreateParamsSetDefault(&params);

    // TODO: should we create discord_client_id.ini? (maybe it is not meant to be viewable by public...)
    char *discord_client_id = SettingsManager::getStringSetting("discord_client_id", "1071527161049124914");
    DiscordClientId parsed_client_id = strtoll(discord_client_id, NULL, 10);
    // printf("discord_client_id %s, parsed %lld\n", discord_client_id, parsed_client_id);
    if (parsed_client_id == LONG_MAX || parsed_client_id == LONG_MIN || !parsed_client_id)
    {
        printf("discord error connect(): failed to parse discord_client_id setting\n");
        return EDiscordResult::DiscordResult_InternalError;
    }
    params.client_id = parsed_client_id;
    params.flags = DiscordCreateFlags_Default;
    params.event_data = &app;
    params.activity_events = &activities_events;
    EDiscordResult result = DiscordCreate(DISCORD_VERSION, &params, &app.core);
    // TODO: it seems it will pass even if id was wrong??, please test
    if (result != EDiscordResult::DiscordResult_Ok)
    {
        printf("discord error connect(): failed to connect to discord client, result returned: %d\n", (int)result);
        memset(&app, 0, sizeof(app));
        return result;
    }

    app.activities = app.core->get_activity_manager(app.core);
    app.application = app.core->get_application_manager(app.core);

    memset(&activity, 0, sizeof(activity));

    strcpy(activity.assets.large_image, "icon");

    // TODO: also put this in some kind of settings?
    strcpy(activity.assets.large_text, "A multiplayer survival game of parenting and civilization building, Join us on discord to play!");
    activity.timestamps.start = time(NULL);

    // force show "Playing a game" because updateActivity will not be called if discord_rich_presence_details.ini=0
    app.activities->update_activity(app.activities, &activity, NULL, OnActivityUpdate);
    app.core->run_callbacks(app.core);
    isHealthy = true;
    printf("discord connect(): connectd\n");
    dLastReport = false;
    return result;
}

void DiscordController::updateActivity(ActivityType activity_type, const char *details, const char *state)
{
    if (!dDisplayGame)
        return;
    printf("discord: updateActivity details=\"%s\" and state=\"%s\"\n", details, state);

    if (!isConnected())
    {
        printf("discord error updateActivity(): we are not connected, this call will be skipped\n");
        return;
    }

    if (details == NULL && state == NULL)
    {
        printf("discord warning updateActivity(): both state and details are NULL\n");
    }

    // TODO: use strncpy
    if (details != NULL)
    {
        strcpy(activity.details, details);
    }
    if (state != NULL)
    {
        strcpy(activity.state, state);
    }
    app.current_activity = activity_type;
    // TODO: try to remove callbacks and see if they are required or not cuz we dont need them.
    app.activities->update_activity(app.activities, &activity, NULL, OnActivityUpdate);
}

void DiscordController::disconnect()
{
    if (isHealthy)
    {
        isHealthy = false;
        app.core->destroy(app.core);
        printf("discord: disconnected, destroyed\n");
    }
}

DiscordController::~DiscordController()
{
    disconnect();
}

EDiscordResult DiscordController::runCallbacks()
{
    if (!dDisplayGame)
        return EDiscordResult::DiscordResult_ApplicationMismatch;
    if (!app.core)
    {
        printf("discord error runCallbacks(): core not initialized, will skip this call\n");
        return EDiscordResult::DiscordResult_ApplicationMismatch;
    }
    if (!isConnected())
    {
        printf("discord error runCallbacks(): we are disconnected from discord, will skip this call\n");
        return EDiscordResult::DiscordResult_ApplicationMismatch;
    }
    EDiscordResult result = app.core->run_callbacks(app.core);
    if (result != EDiscordResult::DiscordResult_Ok)
    {
        printf("discord error runCallbacks(): failed to run callbacks loop, result from run_callbacks(): %d\n", (int)result);
        isHealthy = false; // TODO: where to reset this to true after a failure
    }
    return result;
}

bool DiscordController::isConnected()
{
    return isHealthy;
}

ActivityType DiscordController::getCurrentActivity()
{
    return app.current_activity;
}

void DiscordController::step(DiscordCurrentGamePage page, GamePage *dataPage)
{
    if (!dDisplayGame || !dDisplayDetails)
        return;
    if (!app.core)
    {
        printf("discord error step(): core not initialized, will skip this call\n");
        return;
    }
    if (!isConnected())
    {
        time_t now = time(0);
        if (now - lastReconnectAttempt > 10)
        {
            printf("discord step(): not connected attempting reconnect now\n");
            connect(); // TODO: does this block while connecting?
        }
        return;
    }

    if (page == DiscordCurrentGamePage::LIVING_LIFE_PAGE)
    {
        if (dataPage == NULL)
        {
            printf("discord error step(): dataPage is NULL sig fault imminent.\n");
            fflush(stdout);
        }
        LivingLifePage *livingLifePage = (LivingLifePage *)dataPage;
        LiveObject *ourObject = livingLifePage->getOurLiveObject();
        if (ourObject != NULL)
        {
            char *ourName;
            int ourAge = (int)livingLifePage->getLastComputedAge();
            if (ourObject->name != NULL)
                ourName = autoSprintf("%s", ourObject->name);
            else
                ourName = stringDuplicate("NAMELESS");
            // TODO: not necesarrly that when we have afkEmote means we are really afk!
            char isAfk = ourObject->currentEmot != NULL && getEmotion(afkEmotionIndex) == ourObject->currentEmot;
            char infertileFound, fertileFound;
            char *t1, *t2; // temp swap strings

            t1 = replaceOnce(ourName, "+INFERTILE+", "", &infertileFound);
            delete[] ourName;
            if (!dLastReport || dDisplayDetails != dLastReportDetails || ActivityType::LIVING_LIFE != getCurrentActivity() || dLastReportedName == NULL || 0 != strcmp(dLastReportedName, ourName) || dLastReportedAge != ourAge || dLastReportedFertility != infertileFound || dLastReportedAfk != isAfk)
            {
                t2 = replaceOnce(t1, "+FERTILE+", "", &fertileFound);
                delete[] t1;
                t1 = trimWhitespace(t2);
                delete[] t2;
                ourName = stringDuplicate(strlen(t1) == 0 ? "NAMELESS" : t1);
                delete[] t1;
                if (dLastReportedName != NULL)
                    delete[] dLastReportedName;
                dLastReportedName = stringDuplicate(ourName);
                dLastReportedAge = ourAge;
                dLastReportedFertility = infertileFound;
                dLastReportedAfk = isAfk;
                char ourGender = getObject(ourObject->displayID)->male ? 'M' : 'F';
                char *details = autoSprintf("Living Life, Age %d [%c]%s%s", ourAge, ourGender, infertileFound ? (char *)" [INF]" : (char *)"", isAfk ? (char *)" [AFK]" : (char *)"");
                char *state = autoSprintf("%s", ourName);
                updateActivity(ActivityType::LIVING_LIFE, details, state);
                delete[] details;
                delete[] state;
                delete[] ourName;
                dLastReportDetails = dDisplayDetails;
                dLastReport = true;
            }
            else
            {
                delete[] t1;
            }
        }
    }
    else if (page == DiscordCurrentGamePage::DISONNECTED_PAGE)
    {
        if (!dLastReport || dDisplayDetails != dLastReportDetails || ActivityType::DISCONNECTED != getCurrentActivity())
        {
            updateActivity(ActivityType::DISCONNECTED, "DISCONNECTED!", "");
            dLastReportedName = NULL;
            dLastReportedAge = -1;
            dLastReportDetails = dDisplayDetails;
            dLastReport = true;
        }
    }
    else if (page == DiscordCurrentGamePage::DEATH_PAGE)
    {
        if (!dLastReport || dDisplayDetails != dLastReportDetails || ActivityType::DEATH_SCREEN != getCurrentActivity())
        {
            if (dataPage == NULL)
            {
                printf("discord error step(): dataPage is NULL sig fault imminent.\n");
                fflush(stdout);
            }
            LivingLifePage *livingLifePage = (LivingLifePage *)dataPage;
            LiveObject *ourObject = livingLifePage->getOurLiveObject();
            if (ourObject != NULL)
            {
                char *ourName;
                int ourAge = (int)livingLifePage->getLastComputedAge();
                if (ourObject->name != NULL)
                    ourName = autoSprintf("%s", ourObject->name);
                else
                    ourName = stringDuplicate("NAMELESS");
                delete[] dLastReportedName;
                dLastReportedName = stringDuplicate(ourName);
                dLastReportedAge = ourAge;

                char infertileFound, fertileFound;
                char *t1, *t2; // temp swap strings

                t1 = replaceOnce(ourName, "+FERTILE+", "", &fertileFound);
                delete[] ourName;
                t2 = replaceOnce(t1, "+INFERTILE+", "", &infertileFound);
                delete[] t1;
                t1 = trimWhitespace(t2);
                delete[] t2;
                ourName = stringDuplicate(strlen(t1) == 0 ? "NAMELESS" : t1);
                delete[] t1;
                char ourGender = getObject(ourObject->displayID)->male ? 'M' : 'F';
                char *details = autoSprintf("Died at age %d [%c]%s", ourAge, ourGender);
                char *state = autoSprintf("%s", ourName);
                updateActivity(ActivityType::DEATH_SCREEN, details, state);
                dLastReportedName = stringDuplicate(ourName);
                delete[] details;
                delete[] state;
                delete[] ourName;
            }
            else
            {
                updateActivity(ActivityType::DEATH_SCREEN, "Died", "");
            }
            dLastReportDetails = dDisplayDetails;
            dLastReport = true;
        }
    }
    else if (page == DiscordCurrentGamePage::LOADING_PAGE)
    {
        if (!dLastReport || dDisplayDetails != dLastReportDetails || ActivityType::GAME_LOADING != getCurrentActivity())
        {
            updateActivity(ActivityType::GAME_LOADING, "Loading Game...", "");
            dLastReportDetails = dDisplayDetails;
            dLastReport = true;
        }
    }
    else if (page == DiscordCurrentGamePage::WAITING_TO_BE_BORN_PAGE)
    {
        if (!dLastReport || dDisplayDetails != dLastReportDetails || ActivityType::WAITING_TO_BE_BORN != getCurrentActivity())
        {
            if (userReconnect)
                updateActivity(ActivityType::WAITING_TO_BE_BORN, "Reconnecting!...", "");
            else
                updateActivity(ActivityType::WAITING_TO_BE_BORN, "Waiting to be born...", "");
            dLastReportedName = NULL;
            dLastReportedAge = -1;
            dLastReportDetails = dDisplayDetails;
            dLastReport = true;
        }
    }
    else if (page == DiscordCurrentGamePage::MAIN_MENU_PAGE)
    {
        if (!dLastReport || dDisplayDetails != dLastReportDetails || ActivityType::IN_MAIN_MENU != getCurrentActivity())
        {
            updateActivity(ActivityType::IN_MAIN_MENU, "In Main Menu", "");
            dLastReportedName = NULL;
            dLastReportedAge = -1;
            dLastReportDetails = dDisplayDetails;
            dLastReport = true;
        }
    }
    else if (page == DiscordCurrentGamePage::SETTINGS_PAGE)
    {
        if (!dLastReport || dDisplayDetails != dLastReportDetails || ActivityType::EDITING_SETTINGS != getCurrentActivity())
        {
            updateActivity(ActivityType::EDITING_SETTINGS, "Editing Settings", "");
            dLastReportedName = NULL;
            dLastReportedAge = -1;
            dLastReportDetails = dDisplayDetails;
            dLastReport = true;
        }
    }
    else if (page == DiscordCurrentGamePage::CONNECTION_LOST_PAGE)
    {
        if (!dLastReport || dDisplayDetails != dLastReportDetails || ActivityType::CONNECTION_LOST != getCurrentActivity())
        {
            updateActivity(ActivityType::CONNECTION_LOST, "", "");
            dLastReportedName = NULL;
            dLastReportedAge = -1;
            dLastReportDetails = dDisplayDetails;
            dLastReport = true;
        }
    }
    else
    {
        printf("discord error step(): unhandled DiscordCurrentGamePage parameter value %d. nothing will be updated\n", page);
    }
}
void DiscordController::updateDisplayGame(char newValue)
{
    printf("discord: DisplayGame was changed to %d\n", newValue);
    dDisplayGame = newValue;
    if (!newValue)
        disconnect();
    else
        connect(); // TODO: does this block while connecting?
}
void DiscordController::updateDisplayDetails(char newValue)
{
    printf("discord: DisplayDetails was changed to %d\n", newValue);
    if (!newValue)
    {
        updateActivity(ActivityType::NO_ACTIVITY, "", "");
        dLastReportDetails = newValue;
    }
    dDisplayDetails = newValue;
}