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
#include <Magick++.h>
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
  kOrange,
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
  "🟩",
  "🟧",
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
  "<:gn:1047262141914304633>",
  "<:or:1047262144934182983>",
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
            return (verbly::notion::partMeronyms %=
              verbly::filter("partMeronyms", false,
                subfilter && verbly::filter(
                  verbly::form::id,
                  verbly::filter::comparison::field_does_not_equal,
                  verbly::form::id)));
          } else {
            return (verbly::notion::partHolonyms %=
              verbly::filter("partHolonyms", false,
                subfilter && verbly::filter(
                  verbly::form::id,
                  verbly::filter::comparison::field_does_not_equal,
                  verbly::form::id)));
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
            return (verbly::notion::partHolonyms %=
              verbly::filter("partHolonyms", false,
                subfilter && verbly::filter(
                  verbly::form::id,
                  verbly::filter::comparison::field_does_not_equal,
                  verbly::form::id)));
          } else {
            return (verbly::notion::partMeronyms %=
              verbly::filter("partMeronyms", false,
                subfilter && verbly::filter(
                  verbly::form::id,
                  verbly::filter::comparison::field_does_not_equal,
                  verbly::form::id)));
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
        switch (height) {
          case kBottom: {
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
          }
          case kMiddle: {
            return subfilter;
          }
          default: break; // Never supported.
        }
      } else {
        return (verbly::form::text == "picture");
      }
      break;
    }
    default: break; // Not supported yet.
  }
  return {};
}

class wanderlust {
public:
  explicit wanderlust(const std::string& filename)
  {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line))
    {
      std::string line2;
      if (!std::getline(file, line2))
      {
        throw std::invalid_argument("Wanderlust file is malformed.");
      }

      puzzles_.emplace_back(line, line2);
    }
  }

  std::tuple<std::string, std::string> getPuzzle(std::mt19937& rng) const
  {
    return puzzles_.at(std::uniform_int_distribution<int>(0, puzzles_.size()-1)(rng));
  }

private:
  std::vector<std::tuple<std::string, std::string>> puzzles_;
};

struct puzzle {
  std::string message; // exact text to be posted to discord
  std::string solution; // pre-canonicalized for answer checking
  std::string attachment_name; // only populated for green clues
  std::string attachment_content; // ^
};

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

        sendPuzzle(event.command.get_channel().id);
      }
    });

    bot_->on_message_create([this](const dpp::message_create_t& event) {
      std::lock_guard answer_lock(answers_mutex_);
      uint64_t puzzle_id = static_cast<uint64_t>(event.msg.message_reference.message_id);
      if (answer_by_message_.count(puzzle_id))
      {
        std::string canonical_answer = answer_by_message_[puzzle_id];
        std::string canonical_attempt = hatkirby::lowercase(event.msg.content);
        while (canonical_attempt.find("||") != std::string::npos)
        {
          canonical_attempt.erase(canonical_attempt.find("||"), 2);
        }
        while (canonical_attempt.find(" ") != std::string::npos)
        {
          canonical_attempt.erase(canonical_attempt.find(" "), 1);
        }

        std::cout << "\"" << canonical_attempt << "\"" << std::endl;
        if (canonical_attempt == canonical_answer)
        {
          if (solved_puzzles_.count(puzzle_id))
          {
            bot_->message_add_reaction(event.msg.id, event.msg.channel_id, "✅");
          } else {
            bot_->message_add_reaction(event.msg.id, event.msg.channel_id, "🎉");
            bot_->message_add_reaction(puzzle_id, event.msg.channel_id, "🏁");

            solved_puzzles_.insert(puzzle_id);

            // Submit the score to the scoreboard.
            curl::curl_form form;
            curl::curl_easy easy;

            std::string avatar_url = event.msg.author.get_avatar_url();
            easy.escape(avatar_url);
            std::string user_id_str = std::to_string(static_cast<uint64_t>(event.msg.author.id));
            std::string username = event.msg.member.nickname.empty() ? event.msg.author.username : event.msg.member.nickname;

            // Forms creation
            curl::curl_pair<CURLformoption,std::string> username_form(CURLFORM_COPYNAME,"username");
            curl::curl_pair<CURLformoption,std::string> username_cont(CURLFORM_COPYCONTENTS,username);
            curl::curl_pair<CURLformoption,std::string> pass_form(CURLFORM_COPYNAME,"user_id");
            curl::curl_pair<CURLformoption,std::string> pass_cont(CURLFORM_COPYCONTENTS,user_id_str);
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
          if (canonical_attempt.size() != canonical_answer.size())
          {
            bot_->message_add_reaction(event.msg.id, event.msg.channel_id, "🍂");
          }
        }
      }
    });

    bot_->start();
#endif

    dpp::snowflake channel(config["discord_channel"].as<uint64_t>());

    database_ = std::make_unique<verbly::database>(config["verbly_datafile"].as<std::string>());
    imagenet_ = std::make_unique<imagenet>(config["imagenet"].as<std::string>());
    wanderlust_ = std::make_unique<wanderlust>(config["wanderlust"].as<std::string>());
    fontpath_ = config["font"].as<std::string>();

    scoreboard_endpoint_ = config["scoreboard_endpoint"].as<std::string>();
    scoreboard_secret_code_ = config["scoreboard_secret_code"].as<std::string>();

    for (;;)
    {
      sendPuzzle(channel);

      std::this_thread::sleep_for(std::chrono::hours(4));
    }
  }

private:

  bool isClueTrivial(Height height, Colour colour, const verbly::form& clue, const verbly::form& solution) const
  {
    if (height == kTop && colour == kWhite)
    {
      return !database_->forms((verbly::filter)clue && (verbly::word::synonyms %= solution)).all().empty();
    } else if (height == kBottom && colour == kWhite)
    {
      return !database_->forms((verbly::filter)clue && (verbly::form::pronunciations %= solution)).all().empty();
    } else if (height == kBottom && colour == kBlack)
    {
      return !database_->forms((verbly::filter)clue && (verbly::form::merographs %= solution)).all().empty()
        || !database_->forms((verbly::filter)clue && (verbly::form::holographs %= solution)).all().empty();
    } else if (height == kMiddle && colour == kPurple)
    {
      return clue.getId() == solution.getId();
    } else if (height == kTop && colour == kPurple)
    {
      return clue.getId() == solution.getId();
    }
    return false;
  }

  void generatePuzzle()
  {
    std::cout << "Generating puzzle..." << std::endl;

    std::set<std::tuple<Height, Colour>> filters = {
      {kTop, kPurple},
      {kTop, kWhite},
      {kTop, kRed},
      {kTop, kBlue},
      //{kTop, kYellow},
      {kMiddle, kYellow},
      {kMiddle, kRed},
      {kMiddle, kBlue},
      {kMiddle, kPurple},
      {kMiddle, kGreen},
      {kMiddle, kOrange},
      {kBottom, kWhite},
      {kBottom, kBlack},
      {kBottom, kRed},
      {kBottom, kBlue},
      {kBottom, kGreen},
    };

    std::set<std::tuple<Height, Colour>> expensive_hints = {
      //{kTop, kYellow},
      //{kMiddle, kYellow},
      {kTop, kPurple},
      {kMiddle, kPurple},
      {kBottom, kBlack},
    };

    std::set<std::tuple<Height, Colour>> moderate_hints = {
      {kTop, kRed},
      {kTop, kBlue},
      {kMiddle, kRed},
      {kMiddle, kBlue},
      //{kBottom, kBlack},
    };

    verbly::filter wordFilter =
      (verbly::form::proper == false) &&
      (verbly::form::length >= 3) &&
      (verbly::form::length <= 11) &&
      ((verbly::form::frequency > 2000000) || (verbly::form::complexity > 1));

    verbly::filter cleanFilter =
      !(verbly::word::usageDomains %= (verbly::notion::wnid == 106718862)) // ethnic slurs
      && !(verbly::notion::wnid == 110630093) // "spastic"
      && !(verbly::notion::fullHypernyms %= (verbly::notion::wnid == 100844254)); // sexual activity

    std::unique_ptr<puzzle> genpuzzle;
    for (;;)
    {
      std::cout << "Generating... " << std::endl;
      try
      {
        int hints = 0;
        int non_purple_uses = 0;
        int non_green_uses = 0;
        int expensive_uses = 0;
        int moderate_uses = 0;
        int green_uses = 0;
        int orange_uses = 0;
        bool green_is_bottom = false;
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
              if (colour != kGreen)
              {
                non_green_uses++;
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
                if (height == kBottom)
                {
                  green_is_bottom = true;
                }
              }
              if (colour == kOrange)
              {
                orange_uses++;
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

        if (non_purple_uses < 1 || (non_green_uses < 1 && green_is_bottom))
        {
          std::cout << "No hints (or only purple or only green bottom hints)." << std::endl;
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

        std::string orange_clue;
        std::string orange_solution;
        if (orange_uses > 0)
        {
          std::tie(orange_clue, orange_solution) = wanderlust_->getPuzzle(rng_);
        }

        verbly::filter forwardFilter = cleanFilter && wordFilter;
        for (int i=0; i<static_cast<int>(kHeightCount); i++) {
          Height height = static_cast<Height>(i);
          std::optional<Colour>& colour = parts[i];
          if (!colour.has_value()) {
            continue;
          }
          if (*colour == kOrange)
          {
            forwardFilter &= (verbly::form::text == orange_solution);
          } else {
            forwardFilter &= makeHintFilter(wordFilter, height, *colour, kTowardSolution);
          }
        }

        verbly::form solution = database_->forms(forwardFilter).first();

        std::cout << "Solution decided: " << solution.getText() << std::endl;

        std::array<std::optional<std::string>, kHeightCount> chosenHints;

        bool made_puzzle = false;
        for (int i=0; i<10; i++)
        {
          verbly::filter admissible = cleanFilter && (verbly::form::proper == false) && (verbly::form::length == static_cast<int>(solution.getText().size()));
          std::ostringstream msg_stream;
          bool trivial = false;
          for (int i=0; i<static_cast<int>(kHeightCount); i++) {
            Height height = static_cast<Height>(i);
            std::optional<Colour>& colour = parts[i];
            if (colour.has_value()) {
              if (*colour == kOrange)
              {
                chosenHints[i] = orange_clue;
                admissible &= (verbly::form::text == orange_solution);
              } else {
                verbly::filter questionFilter = makeHintFilter(solution, height, *colour, kTowardQuestion);
                verbly::form questionPart = database_->forms(questionFilter && cleanFilter && wordFilter).first();
                chosenHints[i] = questionPart.getText();

                if (isClueTrivial(height, *colour, questionPart, solution))
                {
                  trivial = true;
                  break;
                }

                admissible &= makeHintFilter(questionPart, height, *colour, kTowardSolution);
              }
            }
          }

          if (trivial)
          {
            std::cout << "Puzzle is trivial." << std::endl;
            continue;
          }

          if (parts[static_cast<int>(kTop)].has_value()
            && !parts[static_cast<int>(kMiddle)].has_value()
            && filters.count({kMiddle, *parts[static_cast<int>(kTop)]}))
          {
            verbly::filter questionFilter =
              makeHintFilter(solution, kMiddle, *parts[static_cast<int>(kTop)], kTowardQuestion)
                && (verbly::form::text == *chosenHints[static_cast<int>(kTop)]);
            if (!database_->forms(questionFilter).all().empty())
            {
              std::cout << "Expanding top to middle" << std::endl;
              parts[static_cast<int>(kMiddle)] = parts[static_cast<int>(kTop)];
              chosenHints[static_cast<int>(kMiddle)] = chosenHints[static_cast<int>(kTop)];
            }
          } else if (!parts[static_cast<int>(kTop)].has_value()
            && parts[static_cast<int>(kMiddle)].has_value()
            && filters.count({kTop, *parts[static_cast<int>(kMiddle)]}))
          {
            verbly::filter questionFilter =
              makeHintFilter(solution, kTop, *parts[static_cast<int>(kMiddle)], kTowardQuestion)
                && (verbly::form::text == *chosenHints[static_cast<int>(kMiddle)]);
            if (!database_->forms(questionFilter).all().empty())
            {
              std::cout << "Expanding middle to top" << std::endl;
              parts[static_cast<int>(kTop)] = parts[static_cast<int>(kMiddle)];
              chosenHints[static_cast<int>(kTop)] = chosenHints[static_cast<int>(kMiddle)];
            }
          }

          for (int i=0; i<static_cast<int>(kHeightCount); i++)
          {
            Height height = static_cast<Height>(i);
            std::optional<Colour>& colour = parts[i];
            std::optional<std::string>& hint = chosenHints[i];
            if (colour.has_value() && hint.has_value())
            {
              msg_stream << COLOUR_EMOTES[*colour] << " " << *hint << std::endl;
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
            genpuzzle = std::make_unique<puzzle>();
            genpuzzle->message = message_text;
            genpuzzle->solution = hatkirby::lowercase(solution.getText());
            while (genpuzzle->solution.find(" ") != std::string::npos)
            {
              genpuzzle->solution.erase(genpuzzle->solution.find(" "), 1);
            }

            if (green_uses > 0)
            {
              generateGreenPuzzle(solution, green_is_bottom, *genpuzzle);
            }

            made_puzzle = true;
            break;
          } else {
            std::cout << "Too many (" << admissibleResults.size() << ") results." << std::endl;
          }
        }
        if (made_puzzle)
        {
          break;
        }
      } catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
      }

      std::cout << "Waiting five seconds then trying again..." << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // generatePuzzle is only called when there is no cached puzzle and there
    // is no other puzzle being generated. therefore, we do not need to worry
    // about a race condition with another generatePuzzle. however, we do not
    // want to interfere with a sendPuzzle. lock the cached puzzle. if
    // sendPuzzle gets it first, it will see that there is no cached puzzle,
    // it will queue its channel, and then it will return because a puzzle is
    // already being generated. if generatePuzzle gets it first and there are
    // no queued channels, it will store the puzzle and return, which means
    // that a waiting sendPuzzle can immediately grab it. if there is a
    // queued channel, generatePuzzle dequeues it and and calls sendPuzzle on
    // it. which sendPuzzle gets the lock is undetermined, but it does not
    // matter.
    std::optional<dpp::snowflake> send_channel;
    {
      std::lock_guard cache_lock(cache_mutex_);
      cached_puzzle_ = std::move(genpuzzle);
      generating_puzzle_ = false;

      if (!queued_channels_.empty())
      {
        send_channel = queued_channels_.front();
        queued_channels_.pop_front();
      }
    }

    if (send_channel)
    {
      sendPuzzle(*send_channel);
    }
  }

  // called when the bot starts (no cached puzzle)
  // called when /newpuzzle is run (maybe a cached puzzle)
  // called at the end of generatePuzzle if channel queue is non-empty (definitely a cached puzzle)
  //
  // if there is a cached puzzle, send it immediately (removes cached puzzle). if not, add it to the channels queue.
  // at this point there is no cached puzzle. if there is no puzzle being generated right now, spin off a thread to do so
  void sendPuzzle(dpp::snowflake channel)
  {
    // lock the cached puzzle so
    // 1) sendPuzzle in another thread doesn't steal our puzzle
    // 2) generatePuzzle in another thread doesn't provide a puzzle
    //    before we can add something to the channel queue
    std::lock_guard cache_lock(cache_mutex_);
    if (cached_puzzle_ != nullptr)
    {
      std::cout << "Sending to " << static_cast<uint64_t>(channel) << std::endl;
#ifdef ENABLE_BOT
      dpp::message message(channel, cached_puzzle_->message);
      if (!cached_puzzle_->attachment_content.empty())
      {
        message.add_file(cached_puzzle_->attachment_name, cached_puzzle_->attachment_content);
      }

      std::string solution = cached_puzzle_->solution;
      bot_->message_create(message, [this, solution](const dpp::confirmation_callback_t& userdata) {
        const auto& posted_msg = std::get<dpp::message>(userdata.value);
        std::lock_guard answer_lock(answers_mutex_);
        if (answer_by_message_.size() > 3000)
        {
          answer_by_message_.clear();
        }
        answer_by_message_[posted_msg.id] = solution;
      });
#endif

      cached_puzzle_.reset();
    } else {
      std::cout << "Queued " << static_cast<uint64_t>(channel) << std::endl;
      queued_channels_.push_back(channel);
    }

    // at this point, we guarantee that there are no puzzles. a generation
    // thread may already be running, though, so we will not spin up
    // another one if there is.
    if (generating_puzzle_)
    {
      return;
    }

    generating_puzzle_ = true;

    std::thread generation_thread([this](){
      generatePuzzle();
    });
    generation_thread.detach();
  }

  void generateGreenPuzzle(const verbly::form& solution, bool is_bottom, puzzle& genpuzzle)
  {
    if (is_bottom)
    {
      verbly::notion notion = database_->notions(
        (verbly::notion::numOfImages > 1) && solution).first();
      auto [image, extension] = imagenet_->getImageForNotion(notion.getId(), rng_);
      if (image.empty())
      {
        throw std::runtime_error("Could not find image for green hint.");
      }

      Magick::Blob inputblob(image.c_str(), image.length());
      Magick::Image inputimg;
      inputimg.read(inputblob);
      inputimg.magick("png");
      Magick::Blob outputblob;
      inputimg.write(&outputblob);

      genpuzzle.attachment_name = "SPOILER_image.png";
      genpuzzle.attachment_content = std::string((const char*) outputblob.data(), outputblob.length());
    } else {
      double fontsize = 72;
      std::string renderWord = solution.getText();

      Magick::Image tester(Magick::Geometry(1, 1), "white");
      tester.font(fontpath_);
      tester.fontPointsize(fontsize);

      Magick::TypeMetric metric;
      tester.fontTypeMetrics(renderWord, &metric);

      double imgHeight = metric.textHeight() * 2;
      double imgWidth = metric.textWidth() * 2;

      Magick::Image result(Magick::Geometry(imgWidth, imgHeight), "white");
      result.font(fontpath_);
      result.fontPointsize(fontsize);
      result.draw(Magick::DrawableText(metric.textWidth() / 2, metric.textHeight() * 1.25, renderWord));
      result.charcoal(2, 2);

      Magick::Image lineimage(Magick::Geometry(imgWidth, imgHeight), "transparent");
      lineimage.strokeColor("black");
      lineimage.fillColor("black");

      int lines = std::uniform_int_distribution<int>(1, 3)(rng_);
      for (int i=0; i<lines; i++) {
        lineimage.strokeWidth(std::uniform_int_distribution<int>(2, 6)(rng_));
        lineimage.draw(Magick::DrawableLine(
          std::uniform_int_distribution<int>(imgWidth / 10, imgWidth / 8)(rng_),
          std::uniform_int_distribution<int>(imgHeight / 10, imgHeight - (imgHeight / 10))(rng_),
          imgWidth - std::uniform_int_distribution<int>(imgWidth / 10, imgWidth / 8)(rng_),
          std::uniform_int_distribution<int>(imgHeight / 10, imgHeight - (imgHeight / 10))(rng_)));
      }

      lineimage.gaussianBlur(std::uniform_int_distribution<int>(2,4)(rng_), std::uniform_int_distribution<int>(1,3)(rng_));
      result.composite(lineimage, 0, 0, Magick::OverCompositeOp);

      result.swirl(std::uniform_int_distribution<int>(45, 100)(rng_));
      result.addNoise(Magick::NoiseType::GaussianNoise);
      result.motionBlur(
        std::uniform_int_distribution<int>(1,4)(rng_),
        std::uniform_int_distribution<int>(1,4)(rng_),
        std::uniform_int_distribution<int>(0, 360)(rng_));

      result.magick("png");

      Magick::Blob outputBlob;
      result.write(&outputBlob);

      genpuzzle.attachment_name = "image.png";
      genpuzzle.attachment_content = std::string((const char*) outputBlob.data(), outputBlob.length());
    }
  }

  std::mt19937& rng_;
  std::unique_ptr<dpp::cluster> bot_;
  std::unique_ptr<verbly::database> database_;
  std::unique_ptr<imagenet> imagenet_;
  std::string scoreboard_endpoint_;
  std::string scoreboard_secret_code_;
  std::unique_ptr<wanderlust> wanderlust_;
  std::string fontpath_;

  std::map<uint64_t, std::string> answer_by_message_;
  std::set<uint64_t> solved_puzzles_;
  std::mutex answers_mutex_;

  bool generating_puzzle_ = false;
  std::unique_ptr<puzzle> cached_puzzle_;
  std::deque<dpp::snowflake> queued_channels_;
  std::mutex cache_mutex_;
};

int main(int argc, char** argv)
{
  Magick::InitializeMagick(nullptr);

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
