#include <dpp/dpp.h>
#include <random>
#include <yaml-cpp/yaml.h>
#include <curl_easy.h>
#include <curl_pair.h>
#include <curl_form.h>
#include <curl_exception.h>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <algorithm>
#include <verbly.h>
#include <json.hpp>
#include <optional>
#include <map>
#include <array>
#include "imagenet.h"

#define ENABLE_BOT

enum Height {
  kTop,
  kMiddle,
  kBottom,
  kHeightCount
};

enum Colour {
  kWhite,
  kBlack,
  kRed,
  kBlue,
  kPurple,
  kBrown,
  kYellow,
  kGreen,
  kColourCount
};

const std::string COLOUR_EMOJIS[kColourCount] = {
  "⬜️",
  "⬛️",
  "🟥",
  "🟦",
  "🟪",
  "🟫",
  "🟨",
  "🟩"
};

const std::string NONE_EMOTE = "<:xx:1047267830535557180>";

const std::string COLOUR_EMOTES[kColourCount] = {
  "<:wt:1047262151032713267>",
  "<:bk:1047262137082445965>",
  "<:rd:1047262147933122560>",
  "<:bl:1047262138202325042>",
  "<:pr:1047262146926489691>",
  "<:bn:1047262139187998790>",
  "<:yw:1047262152781737986>",
  "<:gn:1047262141914304633>"
};

enum FilterDirection {
  kTowardSolution,
  kTowardQuestion
};

verbly::filter makeHintFilter(verbly::filter subfilter, Height height, Colour colour, FilterDirection filter_direction)
{
  switch (colour) {
    case kWhite: {
      switch (height) {
        case kBottom: {
          return (verbly::word::synonyms %= subfilter);
        }
        case kTop: {
          return (verbly::form::pronunciations %=
            verbly::filter("homophones", false,
              (verbly::pronunciation::forms %= (subfilter && verbly::filter(
                verbly::form::id,
                verbly::filter::comparison::field_does_not_equal,
                verbly::form::id)))));
        }
        default: break; // Not supported yet.
      }
      break;
    }
    case kBlack: {
      switch (height) {
        case kBottom: {
          return (verbly::word::antonyms %= subfilter);
        }
        default: break; // Not supported yet.
      }
      break;
    }
    case kBrown: {
      break; // Not supported yet.
    }
    case kRed: {
      switch (height) {
        case kTop: {
          if (filter_direction == kTowardSolution)
          {
            return (verbly::pronunciation::merophones %= subfilter);
          } else {
            return (verbly::pronunciation::holophones %= subfilter);
          }
        }
        case kMiddle: {
          if (filter_direction == kTowardSolution)
          {
            return (verbly::form::merographs %= subfilter);
          } else {
            return (verbly::form::holographs %= subfilter);
          }
        }
        case kBottom: {
          if (filter_direction == kTowardSolution)
          {
            return (verbly::notion::partMeronyms %= subfilter);
          } else {
            return (verbly::notion::partHolonyms %= subfilter);
          }
        }
        default: break; // Not supported yet.
      }
      break;
    }
    case kBlue: {
      switch (height) {
        case kTop: {
          if (filter_direction == kTowardSolution)
          {
            return (verbly::pronunciation::holophones %= subfilter);
          } else {
            return (verbly::pronunciation::merophones %= subfilter);
          }
        }
        case kMiddle: {
          if (filter_direction == kTowardSolution)
          {
            return (verbly::form::holographs %= subfilter);
          } else {
            return (verbly::form::merographs %= subfilter);
          }
        }
        case kBottom: {
          if (filter_direction == kTowardSolution)
          {
            return (verbly::notion::partHolonyms %= subfilter);
          } else {
            return (verbly::notion::partMeronyms %= subfilter);
          }
        }
        default: break; // Not supported yet.
      }
      break;
    }
    case kPurple: {
      switch (height) {
        case kMiddle: {
          return (verbly::form::holographs %=
            verbly::filter("midpurp", false,
              (verbly::form::length >= 4 && (verbly::form::merographs %=
                (subfilter && verbly::filter(
                  verbly::form::id,
                  verbly::filter::comparison::field_does_not_equal,
                  verbly::form::id))))));
        }
        case kTop: {
          return (verbly::pronunciation::holophones %=
            verbly::filter("toppurp", false,
              (verbly::pronunciation::numOfSyllables >= 2 && (verbly::pronunciation::merophones %=
                (subfilter && verbly::filter(
                  verbly::pronunciation::id,
                  verbly::filter::comparison::field_does_not_equal,
                  verbly::pronunciation::id))))));
        }
        default: break; // Not supported yet.
      }
      break;
    }
    case kYellow: {
      switch (height) {
        case kTop: {
          return (verbly::pronunciation::anaphones %= (subfilter && verbly::filter(
                verbly::pronunciation::id,
                verbly::filter::comparison::field_does_not_equal,
                verbly::pronunciation::id)));
        }
        case kMiddle: {
          return (verbly::form::anagrams %= (subfilter && verbly::filter(
                verbly::form::id,
                verbly::filter::comparison::field_does_not_equal,
                verbly::form::id)));
        }
        default: break; // Not supported yet.
      }
      break;
    }
    case kGreen: {
      if (filter_direction == kTowardSolution)
      {
        verbly::filter whitelist =
          (verbly::notion::wnid == 109287968)    // Geological formations
          || (verbly::notion::wnid == 109208496) // Asterisms (collections of stars)
          || (verbly::notion::wnid == 109239740) // Celestial bodies
          || (verbly::notion::wnid == 109277686) // Exterrestrial objects (comets and meteroids)
          || (verbly::notion::wnid == 109403211) // Radiators (supposedly natural radiators but actually these are just pictures of radiators)
          || (verbly::notion::wnid == 109416076) // Rocks
          || (verbly::notion::wnid == 105442131) // Chromosomes
          || (verbly::notion::wnid == 100324978) // Tightrope walking
          || (verbly::notion::wnid == 100326094) // Rock climbing
          || (verbly::notion::wnid == 100433458) // Contact sports
          || (verbly::notion::wnid == 100433802) // Gymnastics
          || (verbly::notion::wnid == 100439826) // Track and field
          || (verbly::notion::wnid == 100440747) // Skiing
          || (verbly::notion::wnid == 100441824) // Water sport
          || (verbly::notion::wnid == 100445351) // Rowing
          || (verbly::notion::wnid == 100446980) // Archery
            // TODO: add more sports
          || (verbly::notion::wnid == 100021939) // Artifacts
          || (verbly::notion::wnid == 101471682) // Vertebrates
            ;

        verbly::filter blacklist =
          (verbly::notion::wnid == 106883725) // swastika
          || (verbly::notion::wnid == 104416901) // tetraskele
          || (verbly::notion::wnid == 102512053) // fish
          || (verbly::notion::wnid == 103575691) // instrument of execution
          || (verbly::notion::wnid == 103829563) // noose
          || (verbly::notion::wnid == 103663910) // life support
            ;

        return subfilter
          && (verbly::notion::fullHypernyms %= whitelist)
          && !(verbly::notion::fullHypernyms %= blacklist)
          && (verbly::notion::partOfSpeech == verbly::part_of_speech::noun)
          && (verbly::notion::numOfImages >= 1);
      } else {
        return (verbly::form::text == "picture");
      }
      break;
    }
    default: break; // Not supported yet.
  }
  return {};
}

class lingo {
public:
  lingo(std::mt19937& rng) : rng_(rng) {}

  void run(const std::string& configpath)
  {
    YAML::Node config = YAML::LoadFile(configpath);

#ifdef ENABLE_BOT
    bot_ = std::make_unique<dpp::cluster>(config["discord_token"].as<std::string>());

    bot_->on_ready([this](const dpp::ready_t& event) {
      if (dpp::run_once<struct register_bot_commands>())
      {
        dpp::slashcommand newcommand("newpuzzle", "Generate a new LINGO puzzle", bot_->me.id);
        bot_->global_command_create(newcommand);
      }
    });

    bot_->on_slashcommand([this](const dpp::slashcommand_t& event) {
      if (event.command.get_command_name() == "newpuzzle") {
        event.thinking(true);

        std::thread genpuzz(&lingo::generatePuzzle, this, event.command.get_channel().id);
        genpuzz.detach();
      }
    });

    bot_->on_message_create([this](const dpp::message_create_t& event) {
      std::lock_guard answer_lock(answers_mutex_);
      uint64_t puzzle_id = static_cast<uint64_t>(event.msg.message_reference.message_id);
      if (answer_by_message_.count(puzzle_id))
      {
        std::string canonical_answer = hatkirby::lowercase(answer_by_message_[puzzle_id]);
        std::string canonical_attempt = hatkirby::lowercase(event.msg.content);
        while (canonical_attempt.find("||") != std::string::npos)
        {
          canonical_attempt.erase(canonical_attempt.find("||"), 2);
        }

        std::cout << "\"" << canonical_attempt << "\"" << std::endl;
        if (canonical_attempt == canonical_answer)
        {
          if (solved_puzzles_.count(puzzle_id))
          {
            bot_->message_add_reaction(event.msg.id, event.msg.channel_id, "✅");
          } else {
            bot_->message_add_reaction(event.msg.id, event.msg.channel_id, "🎉");
            solved_puzzles_.insert(puzzle_id);

            // Submit the score to the scoreboard.
            curl::curl_form form;
            curl::curl_easy easy;

            std::string avatar_url = event.msg.author.get_avatar_url();
            easy.escape(avatar_url);

            // Forms creation
            curl::curl_pair<CURLformoption,std::string> username_form(CURLFORM_COPYNAME,"username");
            curl::curl_pair<CURLformoption,std::string> username_cont(CURLFORM_COPYCONTENTS,event.msg.author.username);
            curl::curl_pair<CURLformoption,std::string> pass_form(CURLFORM_COPYNAME,"user_id");
            curl::curl_pair<CURLformoption,std::string> pass_cont(CURLFORM_COPYCONTENTS,std::to_string(static_cast<uint64_t>(event.msg.author.id)));
            curl::curl_pair<CURLformoption,std::string> av_form(CURLFORM_COPYNAME,"avatar_url");
            curl::curl_pair<CURLformoption,std::string> av_cont(CURLFORM_COPYCONTENTS,avatar_url);
            curl::curl_pair<CURLformoption,std::string> code_form(CURLFORM_COPYNAME,"secret_code");
            curl::curl_pair<CURLformoption,std::string> code_cont(CURLFORM_COPYCONTENTS,scoreboard_secret_code_);

            try {
                // Form adding
                form.add(username_form,username_cont);
                form.add(pass_form,pass_cont);
                form.add(av_form,av_cont);
                form.add(code_form,code_cont);

                // Add some options to our request
                easy.add<CURLOPT_URL>(scoreboard_endpoint_.c_str());
                easy.add<CURLOPT_SSL_VERIFYPEER>(false);
                easy.add<CURLOPT_HTTPPOST>(form.get());
                // Execute the request.
                easy.perform();

            } catch (curl::curl_easy_exception &error) {
                // Otherwise we could print the stack like this:
                error.print_traceback();
            }
          }
        } else {
          bot_->message_add_reaction(event.msg.id, event.msg.channel_id, "❌");
        }
      }
    });

    bot_->start();
#endif

    dpp::snowflake channel(config["discord_channel"].as<uint64_t>());

    database_ = std::make_unique<verbly::database>(config["verbly_datafile"].as<std::string>());
    imagenet_ = std::make_unique<imagenet>(config["imagenet"].as<std::string>());

    scoreboard_endpoint_ = config["scoreboard_endpoint"].as<std::string>();
    scoreboard_secret_code_ = config["scoreboard_secret_code"].as<std::string>();

    for (;;)
    {
      std::thread genpuzz(&lingo::generatePuzzle, this, channel);
      genpuzz.detach();

      std::this_thread::sleep_for(std::chrono::hours(24));
    }
  }

private:

  void generatePuzzle(dpp::snowflake channel)
  {
    std::set<std::tuple<Height, Colour>> filters = {
      {kTop, kPurple},
      {kTop, kWhite},
      {kTop, kRed},
      {kTop, kBlue},
      {kMiddle, kYellow},
      {kMiddle, kRed},
      {kMiddle, kBlue},
      {kMiddle, kPurple},
      {kMiddle, kGreen},
      {kBottom, kWhite},
      {kBottom, kBlack},
      {kBottom, kRed},
      {kBottom, kBlue},
      {kBottom, kGreen},
    };

    std::set<std::tuple<Height, Colour>> expensive_hints = {
      {kTop, kPurple},
      {kMiddle, kPurple},
    };

    std::set<std::tuple<Height, Colour>> moderate_hints = {
      {kTop, kRed},
      {kTop, kBlue},
      {kMiddle, kRed},
      {kMiddle, kBlue},
      {kBottom, kBlack},
    };

    verbly::filter wordFilter = (verbly::form::proper == false);

    verbly::filter cleanFilter =
      !(verbly::word::usageDomains %= (verbly::notion::wnid == 106718862)) // ethnic slurs
      && !(verbly::notion::wnid == 110630093); // "spastic"

    bool generated = false;
    while (!generated)
    {
      std::cout << "Generating... " << std::endl;
      try
      {
        int hints = 0;
        int non_purple_uses = 0;
        int expensive_uses = 0;
        int moderate_uses = 0;
        int green_uses = 0;
        std::array<std::optional<Colour>, kHeightCount> parts;
        for (int height = 0; height < static_cast<int>(kHeightCount); height++) {
          if (std::bernoulli_distribution(0.5)(rng_)) {
            int colour = std::uniform_int_distribution<int>(0, static_cast<int>(kColourCount)-1)(rng_);
            auto combo = std::make_tuple<Height, Colour>(static_cast<Height>(height), static_cast<Colour>(colour));
            if (filters.count(combo)) {
              parts[static_cast<Height>(height)] = static_cast<Colour>(colour);

              hints++;
              if (colour != kPurple)
              {
                non_purple_uses++;
              }
              if (expensive_hints.count(combo))
              {
                expensive_uses++;
              }
              if (moderate_hints.count(combo))
              {
                moderate_uses++;
              }
              if (colour == kGreen)
              {
                green_uses++;
              }

              std::cout << COLOUR_EMOJIS[colour];
            } else {
              std::cout << "▪️";
            }
          } else {
            std::cout << "▪️";
          }
        }
        std::cout << std::endl;

        if (non_purple_uses < 1)
        {
          std::cout << "No hints (or only purple hints)." << std::endl;
          continue;
        }
        if (expensive_uses > 1)
        {
          std::cout << "Too many expensive hints." << std::endl;
          continue;
        }
        if (expensive_uses == 1 && moderate_uses > 0) {
          std::cout << "Moderate hints can't be combined with an expensive hint." << std::endl;
          continue;
        }
        if (green_uses > 1)
        {
          std::cout << "Too many green hints." << std::endl;
          continue;
        }

        verbly::filter forwardFilter = cleanFilter && (verbly::form::proper == false);
        for (int i=0; i<static_cast<int>(kHeightCount); i++) {
          Height height = static_cast<Height>(i);
          std::optional<Colour>& colour = parts[i];
          if (!colour.has_value()) {
            continue;
          }
          forwardFilter &= makeHintFilter(wordFilter, height, *colour, kTowardSolution);
        }

        verbly::form solution = database_->forms(forwardFilter).first();
        verbly::filter admissible = cleanFilter && (verbly::form::proper == false);

        std::cout << "Solution decided: " << solution.getText() << std::endl;

        std::ostringstream msg_stream;
        for (int i=0; i<static_cast<int>(kHeightCount); i++) {
          Height height = static_cast<Height>(i);
          std::optional<Colour>& colour = parts[i];
          if (colour.has_value()) {
            verbly::filter questionFilter = makeHintFilter(solution, height, *colour, kTowardQuestion);
            verbly::form questionPart = database_->forms(questionFilter && cleanFilter).first();
            msg_stream << COLOUR_EMOTES[*colour] << " " << questionPart.getText() << std::endl;

            admissible &= makeHintFilter(questionPart, height, *colour, kTowardSolution);
          } else {
            msg_stream << NONE_EMOTE << std::endl;
          }
        }
        auto byspace = hatkirby::split<std::list<std::string>>(solution.getText(), " ");
        std::list<std::string> lens;
        for (const std::string& wordpart : byspace)
        {
          lens.push_back(std::to_string(wordpart.size()));
        }

        msg_stream << "(" << hatkirby::implode(std::begin(lens), std::end(lens), " ") << ")";

        std::string message_text = msg_stream.str();
        std::cout << message_text << std::endl << std::endl << solution.getText() << std::endl;

        std::vector<verbly::form> admissibleResults = database_->forms(admissible, {}, 10).all();
        if (green_uses > 0 || (admissibleResults.size() <= (hints == 1 ? 2 : 5)))
        {
#ifdef ENABLE_BOT
          dpp::message message(channel, message_text);

          if (green_uses > 0)
          {
            verbly::notion notion = database_->notions(
              (verbly::notion::numOfImages > 1) && solution).first();
            auto [image, extension] = imagenet_->getImageForNotion(notion.getId(), rng_);
            if (image.empty())
            {
              throw std::runtime_error("Could not find image for green hint.");
            }

            message.add_file(std::string("SPOILER_image.") + extension, image);
          }

          bot_->message_create(message, [this, &solution](const dpp::confirmation_callback_t& userdata) {
            const auto& posted_msg = std::get<dpp::message>(userdata.value);
            std::lock_guard answer_lock(answers_mutex_);
            if (answer_by_message_.size() > 3000)
            {
              answer_by_message_.clear();
            }
            answer_by_message_[posted_msg.id] = solution.getText();
          });
#endif

          generated = true;
        } else {
          std::cout << "Too many (" << admissibleResults.size() << ") results." << std::endl;
        }
      } catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
      }

      std::cout << "Waiting five seconds then trying again..." << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  }

  std::mt19937& rng_;
  std::unique_ptr<dpp::cluster> bot_;
  std::unique_ptr<verbly::database> database_;
  std::unique_ptr<imagenet> imagenet_;
  std::map<uint64_t, std::string> answer_by_message_;
  std::set<uint64_t> solved_puzzles_;
  std::mutex answers_mutex_;
  std::string scoreboard_endpoint_;
  std::string scoreboard_secret_code_;
};

int main(int argc, char** argv)
{
  std::random_device randomDevice;
  std::mt19937 rng{randomDevice()};

  if (argc != 2)
  {
    std::cout << "usage: lingo [configfile]" << std::endl;
    return -1;
  }

  lingo lingo(rng);
  lingo.run(argv[1]);

  return 0;
}
