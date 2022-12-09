#include <dpp/dpp.h>
#include <random>
#include <yaml-cpp/yaml.h>
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
  kColourCount
};

const std::string COLOUR_EMOJIS[kColourCount] = {
  "⬜️",
  "⬛️",
  "🟥",
  "🟦",
  "🟪",
  "🟫",
  "🟨"
};

class lingo {
public:
  lingo(std::mt19937& rng) : rng_(rng) {}

  void run(const std::string& configpath)
  {
    YAML::Node config = YAML::LoadFile(configpath);
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
      if (answer_by_message_.count(static_cast<uint64_t>(event.msg.message_reference.message_id)))
      {
        std::string canonical_answer = hatkirby::lowercase(answer_by_message_[event.msg.message_reference.message_id]);
        std::string canonical_attempt = hatkirby::lowercase(event.msg.content);
        while (canonical_attempt.find("||") != std::string::npos)
        {
          canonical_attempt.erase(canonical_attempt.find("||"), 2);
        }

        std::cout << "\"" << canonical_attempt << "\"" << std::endl;
        if (canonical_attempt == canonical_answer)
        {
          bot_->message_add_reaction(event.msg.id, event.msg.channel_id, "✅");
        } else {
          bot_->message_add_reaction(event.msg.id, event.msg.channel_id, "❌");
        }
      }
    });

    bot_->start();

    dpp::snowflake channel(config["discord_channel"].as<uint64_t>());

    database_ = std::make_unique<verbly::database>(config["verbly_datafile"].as<std::string>());

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
      {kTop, kYellow},
      {kTop, kRed},
      {kTop, kBlue},
      {kMiddle, kYellow},
      {kMiddle, kRed},
      {kMiddle, kBlue},
      {kMiddle, kPurple},
      {kBottom, kWhite},
      {kBottom, kBlack},
      {kBottom, kRed},
      {kBottom, kBlue},
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
        std::array<std::optional<Colour>, kHeightCount> parts;
        for (int height = 0; height < static_cast<int>(kHeightCount); height++) {
          if (std::bernoulli_distribution(0.5)(rng_)) {
            int colour = std::uniform_int_distribution<int>(0, static_cast<int>(kColourCount)-1)(rng_);
            if (filters.count({static_cast<Height>(height), static_cast<Colour>(colour)})) {
              parts[static_cast<Height>(height)] = static_cast<Colour>(colour);
              hints++;
              std::cout << COLOUR_EMOJIS[colour];
            } else {
              std::cout << "▪️";
            }
          }
        }
        std::cout << std::endl;

        if (hints < 1) {
          continue;
        }

        verbly::filter forwardFilter = cleanFilter && (verbly::form::proper == false);
        for (int i=0; i<static_cast<int>(kHeightCount); i++) {
          Height height = static_cast<Height>(i);
          std::optional<Colour>& colour = parts[i];
          if (!colour.has_value()) {
            continue;
          }
          switch (*colour) {
            case kWhite: {
              switch (height) {
                case kBottom: {
                  forwardFilter &= (verbly::word::synonyms %= wordFilter);
                  break;
                }
                case kTop: {
                  forwardFilter &= (verbly::form::pronunciations %=
                    verbly::filter("homophones", false,
                      (verbly::pronunciation::forms %= (wordFilter && verbly::filter(
                        verbly::form::id,
                        verbly::filter::comparison::field_does_not_equal,
                        verbly::form::id)))));
                  break;
                }
                default: break; // Not supposed yet.
              }
              break;
            }
            case kBlack: {
              switch (height) {
                case kBottom: {
                  forwardFilter &= (verbly::word::antonyms %= wordFilter);
                  break;
                }
                default: break; // Not supposed yet.
              }
              break;
            }
            case kBrown: {
              switch (height) {
                case kBottom: {
                  forwardFilter &= (verbly::notion::causes %= wordFilter);
                  break;
                }
                default: break; // Not supposed yet.
              }
              break;
            }
            case kRed: {
              switch (height) {
                case kTop: {
                  forwardFilter &= (verbly::pronunciation::holophones %= wordFilter);
                  break;
                }
                case kMiddle: {
                  forwardFilter &= (verbly::form::holographs %= wordFilter);
                  break;
                }
                case kBottom: {
                  forwardFilter &= (verbly::notion::partMeronyms %= wordFilter);
                  break;
                }
                default: break; // Not supposed yet.
              }
              break;
            }
            case kBlue: {
              switch (height) {
                case kTop: {
                  forwardFilter &= (verbly::pronunciation::merophones %= wordFilter);
                  break;
                }
                case kMiddle: {
                  forwardFilter &= (verbly::form::merographs %= wordFilter);
                  break;
                }
                case kBottom: {
                  forwardFilter &= (verbly::notion::partHolonyms %= wordFilter);
                  break;
                }
                default: break; // Not supposed yet.
              }
              break;
            }
            case kPurple: {
              switch (height) {
                case kMiddle: {
                  forwardFilter &= (verbly::form::merographs %= (verbly::form::length >= 4 && (verbly::form::holographs %= wordFilter)));
                  break;
                }
                case kTop: {
                  forwardFilter &= (verbly::pronunciation::rhymes %= wordFilter);
                  break;
                }
                default: break; // Not supposed yet.
              }
              break;
            }
            case kYellow: {
              switch (height) {
                case kTop: {
                  forwardFilter &= (verbly::pronunciation::anaphones %= (wordFilter && verbly::filter(
                        verbly::pronunciation::id,
                        verbly::filter::comparison::field_does_not_equal,
                        verbly::pronunciation::id)));
                  break;
                }
                case kMiddle: {
                  forwardFilter &= (verbly::form::anagrams %= (wordFilter && verbly::filter(
                        verbly::form::id,
                        verbly::filter::comparison::field_does_not_equal,
                        verbly::form::id)));
                  break;
                }
                default: break; // Not supposed yet.
              }
              break;
            }
            default: break; // Not supposed yet.
          }
        }

        verbly::form solution = database_->forms(forwardFilter).first();
        verbly::filter admissable = cleanFilter && (verbly::form::proper == false);

        std::ostringstream msg_stream;
        for (int i=0; i<static_cast<int>(kHeightCount); i++) {
          Height height = static_cast<Height>(i);
          std::optional<Colour>& colour = parts[i];
          if (colour.has_value()) {
            verbly::filter questionFilter;
            switch (*colour) {
              case kWhite: {
                switch (height) {
                  case kBottom: {
                    questionFilter = (verbly::word::synonyms %= solution);
                    break;
                  }
                  case kTop: {
                    questionFilter = (verbly::form::pronunciations %=
                      verbly::filter("homophones", false,
                        (verbly::pronunciation::forms %= ((verbly::filter)solution && verbly::filter(
                          verbly::form::id,
                          verbly::filter::comparison::field_does_not_equal,
                          verbly::form::id)))));
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kBlack: {
                switch (height) {
                  case kBottom: {
                    questionFilter = (verbly::word::antonyms %= solution);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kBrown: {
                switch (height) {
                  case kBottom: {
                    questionFilter = (verbly::notion::effects %= solution);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kBlue: {
                switch (height) {
                  case kTop: {
                    questionFilter = (verbly::pronunciation::holophones %= solution);
                    break;
                  }
                  case kMiddle: {
                    questionFilter = (verbly::form::holographs %= solution);
                    break;
                  }
                  case kBottom: {
                    /*questionFilter = ((verbly::notion::fullMemberHolonyms %= solution)
                      || (verbly::notion::fullPartHolonyms %= solution)
                      || (verbly::notion::fullSubstanceHolonyms %= solution));*/
                    //questionFilter &= !(verbly::notion::words %= solution);
                    questionFilter = (verbly::notion::partMeronyms %= solution);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kRed: {
                switch (height) {
                  case kTop: {
                    questionFilter = (verbly::pronunciation::merophones %= solution);
                    break;
                  }
                  case kMiddle: {
                    questionFilter = (verbly::form::merographs %= solution);
                    break;
                  }
                  case kBottom: {
                    /*questionFilter = ((verbly::notion::fullMemberMeronyms %= solution)
                      || (verbly::notion::fullPartMeronyms %= solution)
                      || (verbly::notion::fullSubstanceMeronyms %= solution));*/
                    questionFilter = (verbly::notion::partHolonyms %= solution);
                    //questionFilter &= !(verbly::notion::words %= solution);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kPurple: {
                switch (height) {
                  case kTop: {
                    questionFilter = (verbly::pronunciation::rhymes %= solution);
                    break;
                  }
                  case kMiddle: {
                    questionFilter = (verbly::form::merographs %= (verbly::form::length >= 4 && (verbly::form::holographs %= solution)));
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kYellow: {
                switch (height) {
                  case kTop: {
                    questionFilter = (verbly::pronunciation::anaphones %= ((verbly::filter)solution && verbly::filter(
                        verbly::pronunciation::id,
                        verbly::filter::comparison::field_does_not_equal,
                        verbly::pronunciation::id)));
                    break;
                  }
                  case kMiddle: {
                    questionFilter = (verbly::form::anagrams %= ((verbly::filter)solution && verbly::filter(
                        verbly::form::id,
                        verbly::filter::comparison::field_does_not_equal,
                        verbly::form::id)));
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              default: break; // Not supposed yet.
            }
            verbly::form questionPart = database_->forms(questionFilter && cleanFilter).first();
            msg_stream << COLOUR_EMOJIS[*colour] << " " << questionPart.getText() << std::endl;

            verbly::filter addedClause = (verbly::form::text == questionPart.getText());

            switch (*colour) {
              case kWhite: {
                switch (height) {
                  case kBottom: {
                    admissable &= (verbly::word::synonyms %= addedClause);
                    break;
                  }
                  case kTop: {
                    admissable &= (verbly::form::pronunciations %=
                      verbly::filter("homophones", false,
                        (verbly::pronunciation::forms %= (addedClause && verbly::filter(
                          verbly::form::id,
                          verbly::filter::comparison::field_does_not_equal,
                          verbly::form::id)))));
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kBlack: {
                switch (height) {
                  case kBottom: {
                    admissable &= (verbly::word::antonyms %= addedClause);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kBrown: {
                switch (height) {
                  case kBottom: {
                    admissable &= (verbly::notion::causes %= addedClause);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kRed: {
                switch (height) {
                  case kTop: {
                    admissable &= (verbly::pronunciation::holophones %= addedClause);
                    break;
                  }
                  case kMiddle: {
                    admissable &= (verbly::form::holographs %= addedClause);
                    break;
                  }
                  case kBottom: {
                    admissable &= (verbly::notion::partMeronyms %= addedClause);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kBlue: {
                switch (height) {
                  case kTop: {
                    admissable &= (verbly::pronunciation::merophones %= addedClause);
                    break;
                  }
                  case kMiddle: {
                    admissable &= (verbly::form::merographs %= addedClause);
                    break;
                  }
                  case kBottom: {
                    admissable &= (verbly::notion::partHolonyms %= addedClause);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kPurple: {
                switch (height) {
                  case kMiddle: {
                    admissable &= (verbly::form::merographs %= (verbly::form::length >= 4 && (verbly::form::holographs %= addedClause)));
                    break;
                  }
                  case kTop: {
                    admissable &= (verbly::pronunciation::rhymes %= addedClause);
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              case kYellow: {
                switch (height) {
                  case kTop: {
                    admissable &= (verbly::pronunciation::anaphones %= (addedClause && verbly::filter(
                          verbly::pronunciation::id,
                          verbly::filter::comparison::field_does_not_equal,
                          verbly::pronunciation::id)));
                    break;
                  }
                  case kMiddle: {
                    admissable &= (verbly::form::anagrams %= (addedClause && verbly::filter(
                          verbly::form::id,
                          verbly::filter::comparison::field_does_not_equal,
                          verbly::form::id)));
                    break;
                  }
                  default: break; // Not supposed yet.
                }
                break;
              }
              default: break; // Not supposed yet.
            }
          } else {
            msg_stream << "▪️" << std::endl;
          }
        }
        msg_stream << "(" << solution.getText().size() << ")";

        std::string message_text = msg_stream.str();
        std::cout << message_text << std::endl << std::endl << solution.getText() << std::endl;

        std::vector<verbly::form> admissableResults = database_->forms(admissable).all();
        if (admissableResults.size() <= 5)
        {
          dpp::message message(channel, message_text);
          bot_->message_create(message, [this, &solution](const dpp::confirmation_callback_t& userdata) {
            const auto& posted_msg = std::get<dpp::message>(userdata.value);
            std::lock_guard answer_lock(answers_mutex_);
            if (answer_by_message_.size() > 3000)
            {
              answer_by_message_.clear();
            }
            answer_by_message_[posted_msg.id] = solution.getText();
          });

          generated = true;
        } else {
          std::cout << "Too many (" << admissableResults.size() << ") results." << std::endl;
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
  std::map<uint64_t, std::string> answer_by_message_;
  std::mutex answers_mutex_;
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
