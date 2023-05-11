#ifndef DISCORD_CONTROLLER_H
#define DISCORD_CONTROLLER_H

#include "discord_game_sdk.h"
#include "GamePage.h"

class DiscordController;
extern DiscordController *discordControllerInstance; // for settings page

typedef enum ActivityType
{
    NO_ACTIVITY = 0,
    GAME_LOADING,
    IN_MAIN_MENU,
    LIVING_LIFE,
    WAITING_TO_BE_BORN,
    DEATH_SCREEN,
    DISCONNECTED,
    EDITING_SETTINGS,
    CONNECTION_LOST,
    // TODO: can add more (death screen, waiting to be born...)
} ActivityType;

typedef enum DiscordCurrentGamePage
{
    NO_PAGE = 0,
    LOADING_PAGE,
    MAIN_MENU_PAGE,
    LIVING_LIFE_PAGE,
    DISONNECTED_PAGE,
    WAITING_TO_BE_BORN_PAGE,
    DEATH_PAGE,
    SETTINGS_PAGE,
    CONNECTION_LOST_PAGE,
} DiscordCurrentGamePage;

typedef struct IDiscordApplication
{
    struct IDiscordCore *core;
    // struct IDiscordUserManager *users;
    // struct IDiscordAchievementManager *achievements;
    struct IDiscordActivityManager *activities;
    // struct IDiscordRelationshipManager *relationships;
    struct IDiscordApplicationManager *application;
    // struct IDiscordLobbyManager *lobbies;
    // DiscordUserId user_id;
    ActivityType current_activity;
} DiscordApplication;

class DiscordController
{

public:
    ~DiscordController();
    DiscordController();
    EDiscordResult connect();
    EDiscordResult runCallbacks();
    void step(DiscordCurrentGamePage page, GamePage *dataPage);
    void updateDisplayGame(char newValue);
    void updateDisplayDetails(char newValue);

protected:
    DiscordApplication app;
    bool isHealthy; // connection is live and we have an instance
    struct DiscordActivity activity;

    ActivityType getCurrentActivity();
    bool isConnected();
    void disconnect();
    void updateActivity(ActivityType activity_type, const char *details, const char *state);
};
#endif // DISCORD_CONTROLLER_H
