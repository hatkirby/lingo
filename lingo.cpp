#include <mastodonpp/mastodonpp.hpp>
#include <random>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <thread>
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

  verbly::database database(config["verbly_datafile"].as<std::string>());

  /*mastodonpp::Instance instance{
    config["mastodon_instance"].as<std::string>(),
    config["mastodon_token"].as<std::string>()};
  mastodonpp::Connection connection{instance};*/

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
    bool puzzleEmpty = true;
    std::array<std::optional<Colour>, kHeightCount> parts;
    for (int height = 0; height < static_cast<int>(kHeightCount); height++) {
      if (std::bernoulli_distribution(0.5)(rng)) {
        int colour = std::uniform_int_distribution<int>(0, static_cast<int>(kColourCount)-1)(rng);
        if (filters.count({static_cast<Height>(height), static_cast<Colour>(colour)})) {
          parts[static_cast<Height>(height)] = static_cast<Colour>(colour);
          puzzleEmpty = false;
        }
      }
    }

    if (puzzleEmpty) {
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
        std::cout << COLOUR_EMOJIS[*colour] << " " << questionPart.getText() << std::endl;
      } else {
        std::cout << "▪️" << std::endl;
      }
    }
    std::cout << "(" << solution.getText().size() << ")" << std::endl << std::endl << solution.getText() << std::endl;


    // We can poll the timeline at most once every five minutes.
    std::this_thread::sleep_for(std::chrono::hours(3));
  }
}
