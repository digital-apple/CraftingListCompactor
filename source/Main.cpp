/*
 * Goal: Compact the ConstructibleObjectMenu's underlying list by grouping entries with the same exact result:
 * 
 * A new step has to be implemented to the crafting process: After grouping entries with the same result into individual placeholder entries, 
 * we must render a new list after selecting them, containing every individual entry that was compacted, but this comes with a few issues:
 * 
 * - We must parse the final ConstructibleObject list after every relevant plugin, such as "Crafting Recipe Distributor" has made it's changes to the data handler array.
 * - The placeholder entry must not have any crafting information, but a description or detail that tells the final user that it is meant to be a "group" of entries.
 * - Once a placeholder entry gets selected, we must render a new list containing only the relevant (grouped) entries, respecting the game's rules to disable (greyout) specific entries.
 * - We must account for plugins that skip the confirmation box such as "Yes Im Sure".
 * - Pressing the return button inside the grouped list should render the original list, instead of quitting from the menu.
 */



void InitializeLogger()
{
    auto path = SKSE::log::log_directory();
   
    if (!path) { return; }

    const auto plugin = SKSE::PluginDeclaration::GetSingleton();
    *path /= std::format("{}.log", plugin->GetName());

    std::vector<spdlog::sink_ptr> sinks{
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true),
        std::make_shared<spdlog::sinks::msvc_sink_mt>()
    };

    auto logger = std::make_shared<spdlog::logger>("global", sinks.begin(), sinks.end());
   
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::move(logger));
    spdlog::set_pattern("[%^%L%$] %v");
}

std::unordered_map<RE::FormID, std::vector<RE::FormID>> m;

void HandleMessage(SKSE::MessagingInterface::Message* a_message)
{
    switch (a_message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        {
            INFO("Hello, {}!", SKSE::PluginDeclaration::GetSingleton()->GetAuthor());

            const auto data = RE::TESDataHandler::GetSingleton();

            if (data) {
                const auto& arr = data->GetFormArray<RE::BGSConstructibleObject>();

                for (const auto& i : arr) {
                    const auto id = i->createdItem ? i->createdItem->GetFormID() : 0x0;

                    if (m.contains(id)) {
                        auto& vec = m.at(id);

                        vec.push_back(i->GetFormID());
                    } else {
                        m.emplace(id, std::vector{ i->GetFormID() });
                    }
                }

                for (const auto& i : m) {
                    INFO("K: <{:X}>", i.first);

                    for (const auto& j : i.second) {
                        INFO("V: <{:X}>", j);
                    }

                    INFO("");
                }
            }
        }
        break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    InitializeLogger();

    SKSE::Init(a_skse);

    const auto messaging_interface = SKSE::GetMessagingInterface();

    if (!messaging_interface) { stl::report_and_fail("Failed to communicate with the messaging interface!"); }

    messaging_interface->RegisterListener(HandleMessage);

    return true;
}
