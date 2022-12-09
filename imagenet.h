#ifndef IMAGENET_H_41BE424F
#define IMAGENET_H_41BE424F

#include <string>
#include <filesystem>
#include <random>
#include <tuple>

class imagenet {
public:

  explicit imagenet(std::string path);

  // returns bytedata, extension
  std::tuple<std::string, std::string> getImageForNotion(int notion_id, std::mt19937& rng) const;

private:

  std::filesystem::path path_;
};

#endif /* end of include guard: IMAGENET_H_41BE424F */
