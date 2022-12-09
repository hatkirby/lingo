#include "imagenet.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <curl_easy.h>
#include <curl_header.h>

imagenet::imagenet(std::string path) : path_(path) {}

std::tuple<std::string, std::string> imagenet::getImageForNotion(int notion_id, std::mt19937& rng) const
{
  std::filesystem::path filename = path_ / std::to_string(notion_id);
  if (!std::filesystem::exists(filename))
  {
    throw std::invalid_argument(std::string("File does not exist: ") + std::string(filename));
  }

  std::ifstream file(filename);
  std::string line;
  std::vector<std::string> urls;
  while (std::getline(file, line))
  {
    if (!line.empty())
    {
      urls.push_back(line);
    }
  }

  std::string output;
  std::string extension;
  while (!urls.empty())
  {
    int index = std::uniform_int_distribution<int>(0, urls.size()-1)(rng);
    std::string url = urls.at(index);
    urls.erase(std::begin(urls) + index);

    // willyfogg.com is a thumbnail generator known to return 200 even if the target image no longer exists
    if (url.find("willyfogg.com/thumb.php") != std::string::npos)
    {
      continue;
    }

    // Accept string from Google Chrome
    std::string accept = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
    curl::curl_header headers;
    headers.add(accept);

    std::ostringstream imgbuf;
    curl::curl_ios<std::ostringstream> imgios(imgbuf);
    curl::curl_easy imghandle(imgios);

    imghandle.add<CURLOPT_HTTPHEADER>(headers.get());
    imghandle.add<CURLOPT_URL>(url.c_str());
    imghandle.add<CURLOPT_CONNECTTIMEOUT>(30);
    imghandle.add<CURLOPT_TIMEOUT>(300);

    try
    {
      imghandle.perform();
    } catch (const curl::curl_easy_exception& error) {
      error.print_traceback();

      continue;
    }

    if (imghandle.get_info<CURLINFO_RESPONSE_CODE>().get() != 200)
    {
      continue;
    }

    std::string content_type = imghandle.get_info<CURLINFO_CONTENT_TYPE>().get();
    if (content_type.substr(0, 6) != "image/")
    {
      continue;
    }

    output = imgbuf.str();
    extension = url.substr(url.rfind(".") + 1);
    break;
  }

  if (output.empty())
  {
    throw std::invalid_argument(std::string("No valid urls found for ") + std::string(filename));
  }

  return {output, extension};
}
