#include <dpp/dpp.h>
#include <random>
#include <yaml-cpp/yaml.h>
#include <iostream>
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

int main(int argc, char** argv)
{
  std::random_device randomDevice;
  std::mt19937 rng{randomDevice()};

  if (argc != 2)
  {
    std::cout << "usage: lingo [configfile]" << std::endl;
    return -1;
  }

  std::string configfile(argv[1]);
  YAML::Node config = YAML::LoadFile(configfile);

  std::map<uint64_t, std::string> answer_by_message;
  std::mutex answers_mutex;

  dpp::cluster bot(config["discord_token"].as<std::string>());
  bot.on_message_create([&bot, &answers_mutex, &answer_by_message](const dpp::message_create_t& event) {
    std::lock_guard answer_lock(answers_mutex);
    if (answer_by_message.count(event.msg.message_reference.message_id))
    {
      std::string canonical_answer = hatkirby::lowercase(answer_by_message[event.msg.message_reference.message_id]);
      std::string canonical_attempt = hatkirby::lowercase(event.msg.content);
      while (canonical_attempt.find("||") != std::string::npos)
      {
        canonical_attempt.erase(canonical_attempt.find("||"), 2);
      }

      if (canonical_attempt == canonical_answer)
      {
        bot.message_add_reaction(event.msg.id, event.msg.channel_id, "✅");
      } else {
        bot.message_add_reaction(event.msg.id, event.msg.channel_id, "❌");
      }
    }
  });
  bot.start();

  dpp::snowflake channel(config["discord_channel"].as<uint64_t>());

  verbly::database database(config["verbly_datafile"].as<std::string>());

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

  for (;;)
  {
    try
    {
      int hints = 0;
      std::array<std::optional<Colour>, kHeightCount> parts;
      for (int height = 0; height < static_cast<int>(kHeightCount); height++) {
        if (std::bernoulli_distribution(0.5)(rng)) {
          int colour = std::uniform_int_distribution<int>(0, static_cast<int>(kColourCount)-1)(rng);
          if (filters.count({static_cast<Height>(height), static_cast<Colour>(colour)})) {
            parts[static_cast<Height>(height)] = static_cast<Colour>(colour);
            hints++;
          }
        }
      }

      if (hints < 2) {
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

      verbly::form solution = database.forms(forwardFilter).first();

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
          verbly::form questionPart = database.forms(questionFilter && cleanFilter).first();
          msg_stream << COLOUR_EMOJIS[*colour] << " " << questionPart.getText() << std::endl;
        } else {
          msg_stream << "▪️" << std::endl;
        }
      }
      msg_stream << "(" << solution.getText().size() << ")";

      std::string message_text = msg_stream.str();
      std::cout << message_text << std::endl << std::endl << solution.getText() << std::endl;

      dpp::message message(channel, message_text);
      bot.message_create(message, [&answers_mutex, &answer_by_message, &solution](const dpp::confirmation_callback_t& userdata) {
        const auto& posted_msg = std::get<dpp::message>(userdata.value);
        std::lock_guard answer_lock(answers_mutex);
        if (answer_by_message.size() > 3000)
        {
          answer_by_message.clear();
        }
        answer_by_message[posted_msg.id] = solution.getText();
      });

      std::this_thread::sleep_for(std::chrono::hours(3));
    } catch (const std::exception& ex) {
      std::cout << ex.what() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::minutes(1));
  }
}
