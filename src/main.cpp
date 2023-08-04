#include "main.hpp"

#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/SharedCoroutineStarter.hpp"

#include "UnityEngine/WaitForSeconds.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"

#include "HMUI/TitleViewController.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"

#include "custom-types/shared/coroutine.hpp"

#include "ModConfig.hpp"

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup


TMPro::TextMeshProUGUI* title;
UnityEngine::GameObject* canvas;

MAKE_HOOK_MATCH(MainMenuUIHook, &GlobalNamespace::MainMenuViewController::DidActivate, void, GlobalNamespace::MainMenuViewController
    * self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    // Run the original method before our code.
    // Note, you can run the original method after our code if you want to change arguments.
    MainMenuUIHook(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    // Remove ads
    UnityEngine::UI::Button* promo = self->musicPackPromoButton;
    UnityEngine::GameObject* promoObj = promo->get_gameObject()->get_transform()->get_parent()->get_gameObject();
    if(getModConfig().VariableBlockAds.GetValue())
        promoObj->SetActive(false);
    if(!getModConfig().VariableBlockAds.GetValue())
        promoObj->SetActive(true);

    //Find glow lines
    // UnityEngine::GameObject::Find("GlowLines")
    // ->SetActive(false);
    // UnityEngine::GameObject::Find("GlowLines (1)")
    // ->SetActive(false);

    // Find logo
    UnityEngine::GameObject* logo = UnityEngine::GameObject::Find("Logo");

    // Delete children(we need logo for later)
    for(int i = 0; i < logo->get_transform()->get_childCount(); i++){
        logo->get_transform()->GetChild(i)->get_gameObject()->SetActive(false);
    }

    canvas = QuestUI::BeatSaberUI::CreateCanvas();
    canvas->get_transform()->SetParent(logo->get_transform(), false);

    auto center = TMPro::TextAlignmentOptions::_get_Center();

    title = QuestUI::BeatSaberUI::CreateText(canvas->get_transform(), getModConfig().VariableTitleText.GetValue());
    title->set_fontSize(getModConfig().VariableTitleSize.GetValue());
    title->set_alignment(center);

}


custom_types::Helpers::Coroutine startUpdatingTitle(){
    while(true){
        title->SetText(getModConfig().VariableTitleText.GetValue());
        title->set_fontSize(getModConfig().VariableTitleSize.GetValue());
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForSeconds::New_ctor(0.1));
    }
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling)
{

    if(!firstActivation) return;

    GlobalNamespace::SharedCoroutineStarter::get_instance()->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(startUpdatingTitle()));

    UnityEngine::GameObject* container = QuestUI::BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());
    
    QuestUI::IncrementSetting* fontSize = AddConfigValueIncrementFloat(container->get_transform(), getModConfig().VariableTitleSize, 1, 5, 1, 200);
    HMUI::InputFieldView* fontText = AddConfigValueInputString(container->get_transform(), getModConfig().VariableTitleText);
    UnityEngine::UI::Toggle* ads = AddConfigValueToggle(container->get_transform(), getModConfig().VariableBlockAds);
}


// Loads the config from disk using our modInfo, then returns it for use
// other config tools such as config-utils don't use this config, so it can be removed if those are in use
Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load();
    getLogger().info("Loaded TitleSaber!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();


    // Load config
    getModConfig().Init(modInfo);



    getLogger().info("Installing hooks...");

    // Install our hooks


    // QuestUI
    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);
    QuestUI::Register::RegisterMainMenuModSettingsViewController(modInfo, DidActivate);


    INSTALL_HOOK(getLogger(), MainMenuUIHook);

    getLogger().info("Installed all hooks!");
}