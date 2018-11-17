#include "pch.h"
#include "vendor\CLI11.hpp"
#include "Sio.h"

// TODO: Custom types that are shown as HEX? (CLI11 outputs data as decimal, and it'd
// be nicer in hex, but can I seriously be arsed to do anything about that?)

struct ConfigOptions {
    std::uint32_t colour_1;
    std::uint32_t colour_2;
    std::uint32_t colour_3;
    std::uint32_t colour_4;
    std::vector<std::string> invert_channels;
    bool breathing_mode;
    std::uint16_t step_duration;
    std::uint8_t flash_speed;
    std::vector<std::string> fade_in_channels;
};

void config_cmd_callback(ConfigOptions &cfg_options, bool ignore_mb_check)
{
    try {
        Sio sio (ignore_mb_check);

        sio.set_colour(Sio::ColourIndex::Colour1, cfg_options.colour_1);
        sio.set_colour(Sio::ColourIndex::Colour2, cfg_options.colour_2);
        sio.set_colour(Sio::ColourIndex::Colour3, cfg_options.colour_3);
        sio.set_colour(Sio::ColourIndex::Colour4, cfg_options.colour_4);

        sio.set_channel_inverted(Sio::Channel::R, 
                                 std::find(cfg_options.invert_channels.begin(), cfg_options.invert_channels.end(), "r") != cfg_options.invert_channels.end());
        sio.set_channel_inverted(Sio::Channel::G, 
                                 std::find(cfg_options.invert_channels.begin(), cfg_options.invert_channels.end(), "g") != cfg_options.invert_channels.end());
        sio.set_channel_inverted(Sio::Channel::B, 
                                 std::find(cfg_options.invert_channels.begin(), cfg_options.invert_channels.end(), "b") != cfg_options.invert_channels.end());

        sio.set_breathing_mode_enabled(cfg_options.breathing_mode);
        
        sio.set_step_duration(cfg_options.step_duration);

        // Breathing and flashing don't work at the same time
        if (cfg_options.breathing_mode && cfg_options.flash_speed != 0) {
            std::cout << "Warning: breathing mode and flashing mode are mutually exclusive." << std::endl;
        }

        sio.set_flash_speed(cfg_options.flash_speed);

        sio.set_fade_in_enabled(Sio::Channel::R, 
                                 std::find(cfg_options.fade_in_channels.begin(), cfg_options.fade_in_channels.end(), "r") != cfg_options.fade_in_channels.end());
        sio.set_fade_in_enabled(Sio::Channel::G, 
                                 std::find(cfg_options.fade_in_channels.begin(), cfg_options.fade_in_channels.end(), "g") != cfg_options.fade_in_channels.end());
        sio.set_fade_in_enabled(Sio::Channel::B, 
                                 std::find(cfg_options.fade_in_channels.begin(), cfg_options.fade_in_channels.end(), "b") != cfg_options.fade_in_channels.end());
    }
    catch (Sio::Exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

void disable_cmd_callback(bool ignore_mb_check)
{
    try {
        Sio sio (ignore_mb_check);

        sio.set_led_disabled();
    }
    catch (Sio::Exception &e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char **argv)
{
    CLI::App app("RGB LED controller for certain MSI boards", "MSIRGB");
    app.require_subcommand();

    app.get_formatter()->column_width(50); // Formats the help message

    //
    //

    bool ignore_mb_check;
    app.add_flag("--ignore-check", ignore_mb_check, "Skip checking compatibility with MB (at your own risk)");

    //

    ConfigOptions cfg_options;
    CLI::App *cfg_subcmd = app.add_subcommand("config", 
                                              "Applies the specified options to the LEDs (and enables them if they're disabled)")
        ->callback([&cfg_options, &ignore_mb_check]()
            {
                config_cmd_callback(cfg_options, ignore_mb_check);
            }
    );

    auto is_valid_channel = [](const std::string &str) -> std::string
        {
            if (str != "r" && str != "g" && str != "b")
                return std::string("Invalid channel: it must be one or more values of the list ['r', 'g', 'b']");
            else
                return std::string();
        };

    cfg_subcmd->add_option("COLOUR_1", 
                           cfg_options.colour_1, 
                           "Value for colour 1")
        ->check(CLI::Range(Sio::COLOUR_MAX_VALUE))
        ->required();

    cfg_subcmd->add_option("COLOUR_2", 
                           cfg_options.colour_2, 
                           "Value for colour 2")
        ->check(CLI::Range(Sio::COLOUR_MAX_VALUE))
        ->required();

    cfg_subcmd->add_option("COLOUR_3", 
                           cfg_options.colour_3, 
                           "Value for colour 3")
        ->check(CLI::Range(Sio::COLOUR_MAX_VALUE))
        ->required();

    cfg_subcmd->add_option("COLOUR_4", 
                           cfg_options.colour_4, 
                           "Value for colour 4")
        ->check(CLI::Range(Sio::COLOUR_MAX_VALUE))
        ->required();

    cfg_subcmd->add_option("--invert", 
                           cfg_options.invert_channels, 
                           "Invert the specified channel(s)")
        ->type_name("[one or more: 'r', 'g', 'b']")
        ->check(is_valid_channel);

    cfg_subcmd->add_flag("-b,--breathe", 
                         cfg_options.breathing_mode, 
                         "Enable or disable breathing mode");

    cfg_subcmd->add_option("-d,--duration", 
                           cfg_options.step_duration, 
                           "The interval of time (arbitrary unit) between each change of colour")
        ->check(CLI::Range(Sio::STEP_DURATION_MAX_VALUE))
        ->required();

    cfg_subcmd->add_option("-f,--flash", 
                           cfg_options.flash_speed, 
                           "Speed of flashing mode (0 means disabled, 6 means slowest)")
        ->check(CLI::Range(Sio::FLASHING_MAX_VALUE))
        ->required();

    // TODO: What to do about this? Only supported on some boards?
    // Should I remove it, figure out which MBs even support this,
    // or go all in and add rainbow mode support as well, even though
    // it's also only supported in some boards?
    cfg_subcmd->add_option("--fade-in", 
                           cfg_options.fade_in_channels, 
                           "Enable fade-in effect for the specified channel(s) (only supported on some boards)")
        ->type_name("[one or more: 'r', 'g', 'b']")
        ->check(is_valid_channel);

    //

    app.add_subcommand("disable", "Disable the LEDs")
        ->callback([&ignore_mb_check]()
            {
                disable_cmd_callback(ignore_mb_check);
            }
    );

    //
    //

    try {
        app.parse(argc, argv);
        return 0;
    }
    catch (const CLI::ParseError &e) {
        return app.exit(e);
    }
}