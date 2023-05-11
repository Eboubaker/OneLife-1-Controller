#include "GamePage.h"

#include "TextButton.h"
#include "CheckboxButton.h"
#include "RadioButtonSet.h"
#include "ValueSlider.h"
#include "SoundUsage.h"
#include "DropdownList.h"
#include "Background.h"


#include "minorGems/ui/event/ActionListener.h"




class SettingsPage : public GamePage, public ActionListener {
        
    public:
        
        SettingsPage();
        ~SettingsPage();
        

        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );

        virtual void step();

        virtual void actionPerformed( GUIComponent *inTarget );

        
        virtual void makeActive( char inFresh );
        virtual void makeNotActive();
		virtual void updatePage();
        virtual void checkRestartRequired();

    protected:
        
        int mOldFullscreenSetting;
        int mOldBorderlessSetting;
        int mEnableNudeSetting;
        int mEnableFOVSetting;
        int mEnableKActionsSetting;
        int mEnableCenterCameraSetting;
        int mDiscordRichPresenceSetting;
        int mDiscordRichPresenceDetailsSetting;

        int mPage;

        SoundUsage mTestSound;

        double mMusicStartTime;


        Background mBackground;

        // Left Pane
        TextButton mRestartButton;
        
        TextButton mGameplayButton;
        TextButton mControlButton;
        TextButton mScreenButton;
        TextButton mSoundButton;
        TextButton mDiscordButton;
        TextButton mBackButton;
        
        TextButton mEditAccountButton;

        // Gameplay
		CheckboxButton mEnableFOVBox;
		CheckboxButton mEnableCenterCameraBox;
		CheckboxButton mEnableNudeBox;
        
        CheckboxButton mUseCustomServerBox;
        TextField mCustomServerAddressField;
        TextField mCustomServerPortField;
        TextButton mCopyButton;
        TextButton mPasteButton;
        
        // Control
		CheckboxButton mEnableKActionsBox;
        RadioButtonSet *mCursorModeSet;
        ValueSlider mCursorScaleSlider;

        // Screen
        TextButton mRedetectButton;
        CheckboxButton mFullscreenBox;
        CheckboxButton mBorderlessBox;
        
        // Sound
        ValueSlider mMusicLoudnessSlider;
        ValueSlider mSoundEffectsLoudnessSlider;

        // Discord
        CheckboxButton mEnableDiscordRichPresence;
        CheckboxButton mEnableDiscordRichPresenceDetails;
    };
