#include <boost/regex.hpp>
#include <iostream>
#include <cmath>
class hmc_sim;

class hmc_graphviz {
private:
    bool similar_floats(float a, float b) {
      return std::fabs(a - b) < std::numeric_limits<float>::epsilon();
    }

    std::string get_content_from_file(const char* filename);
    void parse_content_and_set(hmc_sim *sim, std::string content);

    unsigned int extract_id_from_string(std::string str) {
      std::string s = boost::regex_replace(
          str,
          boost::regex("[^0-9]*([0-9]+).*"),
          std::string("\\1")
          );
      return std::stoi(s);
    }

public:
    hmc_graphviz(hmc_sim *sim, const char* graphviz_filename = nullptr);
    ~hmc_graphviz(void) {}
};
