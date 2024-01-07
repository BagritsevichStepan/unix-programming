#include <vector>
#include <map>
#include <regex>
#include <cassert>

int string_to_int(const std::string& str, int default_value = 0) {
    return !str.empty() ? std::stoi(str) : default_value;
}

std::string& appendIf(std::string& str, bool condition, const std::string& suffix) {
    if (condition) {
        str.append(suffix);
    }
    return str;
}

std::string derivative(std::string polynomial) {
    if (polynomial.at(0) != '-' && polynomial.at(0) != '+') {
        polynomial.insert(0, 1, '+');
    }

    std::regex reg(R"((\+|-)(\d*)\*?(x?)\^?(\d*))");

    const std::vector<std::smatch> matches{
            std::sregex_iterator{polynomial.begin(), polynomial.end(), reg},
            std::sregex_iterator{}
    };

    std::map<int, int> constant_by_pow;
    for (auto& match : matches) {
        int constant = string_to_int(match.str(2), 1);
        if (match.str(1) == "-") {
            constant *= -1;
        }

        const int min_pow = !match.str(3).empty() ? 1 : 0;
        const int pow = string_to_int(match.str(4), min_pow);

        constant_by_pow[pow] += constant;
    }

    std::string result;
    for (auto it = constant_by_pow.rbegin(); it != constant_by_pow.rend(); ++it) {
        if (it->first == 0) {
            continue;
        }

        const int pow = it->first - 1;
        const int constant = it->first * it->second;

        const bool add_x = pow > 0;
        const bool add_constant = !(add_x && std::abs(constant) == 1);

        appendIf(result, constant >= 0 && !result.empty(), "+");
        appendIf(result, add_constant, std::to_string(constant));

        if (add_x) {
            appendIf(result, add_constant, "*").append("x");
            appendIf(result, pow > 1, "^" + std::to_string(pow));
        }
    }

    return result;
}

int main() {
    assert(derivative("x^2+x") == "2*x+1");
    assert(derivative("2*x^100+100*x^2") == "200*x^99+200*x");
    assert(derivative("x^10000+x+1") == "10000*x^9999+1");
    assert(derivative("-x^2-x^3") == "-3*x^2-2*x");
    assert(derivative("x+x+x+x+x+x+x+x+x+x") == "10");
    return 0;
}

