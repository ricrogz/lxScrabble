//
// Created by invik on 15/09/19.
//

#include <algorithm>
#include <fstream>
#include <sstream>

#include "scoreboard.hpp"

Scoreboard::Scores::Scores(unsigned long week, unsigned long total)
    : m_week{week}, m_total{total}
{
}
void Scoreboard::Scores::clear_week()
{
    m_week = 0ul;
}
void Scoreboard::Scores::clear_total()
{
    m_total = 0ul;
}
void Scoreboard::Scores::clear_all()
{
    m_week = 0ul;
    m_total = 0ul;
}
unsigned long Scoreboard::Scores::get_week() const
{
    return m_week;
}
unsigned long Scoreboard::Scores::get_total() const
{
    return m_total;
}
void Scoreboard::Scores::add(unsigned long score)
{
    m_week += score;
    m_total += score;
}
std::function<void(Scoreboard::Scores&)>
Scoreboard::Scores::get_clear_method(Scoreboard::Type which)
{
    switch (which) {
    case Scoreboard::Type::All:
        return &Scoreboard::Scores::clear_all;
    case Scoreboard::Type::Total:
        return &Scoreboard::Scores::clear_total;
    case Scoreboard::Type::Week:
    default:
        return &Scoreboard::Scores::clear_week;
    }
}
std::function<unsigned long(const Scoreboard::Scores&)>
Scoreboard::Scores::get_score_getter(Scoreboard::Type which)
{
    switch (which) {
    case Scoreboard::Type::Total:
        return &Scoreboard::Scores::get_total;
    case Scoreboard::Type::Week:
    default:
        return &Scoreboard::Scores::get_week;
    }
}

/****************************************************************/

Scoreboard::Scoreboard(std::string scorefile)
    : m_scorefile{std::move(scorefile)}
{
}

void Scoreboard::update_top(std::vector<std::string>& top, Type which,
                            const std::string& player)
{

    auto score_getter = Scores::get_score_getter(which);

    auto comp = [this, &score_getter](const std::string& a,
                                      const std::string& b) {
        return score_getter(m_players.at(a)) > score_getter(m_players.at(b));
    };

    auto current_pos = std::find(top.begin(), top.end(), player);
    auto insert_pos = std::lower_bound(top.begin(), top.end(), player, comp);

    if (current_pos != top.end()) {
        if (current_pos == insert_pos) {
            return;
        } else {
            top.erase(current_pos);
        }
    }

    if (abs(std::distance(top.begin(), insert_pos)) < TOP_SIZE) {
        top.insert(insert_pos, player);
        if (top.size() > TOP_SIZE) {
            top.erase(top.begin() + TOP_SIZE, top.end());
        }
    }
}

Scoreboard* Scoreboard::read_scoreboard(std::string scores_file)
{
    static Scoreboard sb{scores_file};

    if (sb.m_players.empty()) {
        auto f_in = std::ifstream{std::move(scores_file)};
        if (!f_in.good()) {
            std::stringstream ss;
            ss << "Unable to open file '" << scores_file << "' to read scores.";
            throw std::ifstream::failure(ss.str());
        }

        while (!f_in.eof()) {
            auto player = std::string{};
            auto week_score = 0ul;
            auto alltime_score = 0ul;

            f_in >> player >> alltime_score >> week_score;

            if (player.empty()) {
                continue;
            }

            sb.m_players[player] = {week_score, alltime_score};

            if (week_score > 0ul) {
                sb.update_top(sb.m_week_top, Type::Week, player);
            }
            if (alltime_score > 0ul) {
                sb.update_top(sb.m_total_top, Type::Total, player);
            }
        }
        f_in.close();
    }

    return &sb;
}

void Scoreboard::save() const
{
    auto f_out = std::ofstream{m_scorefile};
    if (!f_out.good()) {
        std::stringstream ss;
        ss << "Unable to open file '" << m_scorefile << "' to write scores.";
        throw std::ifstream::failure(ss.str());
    }

    auto weekly_score_getter = Scores::get_score_getter(Type::Week);
    auto total_score_getter = Scores::get_score_getter(Type::Total);

    for (const auto& score : m_players) {
        f_out << score.first << ' ' << total_score_getter(score.second) << ' '
              << weekly_score_getter(score.second) << '\n';
    }
    f_out.close();
}

void Scoreboard::add_score(const std::string& player, unsigned long score)
{
    auto& scores = m_players[player];
    scores.add(score);

    update_top(m_week_top, Type::Week, player);
    update_top(m_total_top, Type::Total, player);

    save();
}

unsigned long Scoreboard::get_score(const std::string& player,
                                    Scoreboard::Type which) const
{
    if (m_players.count(player) == 0) {
        return 0ul;
    }

    auto getter = Scores::get_score_getter(which);
    return getter(m_players.at(player));
}

void Scoreboard::clear(Scoreboard::Type which)
{
    auto clear_func = Scores::get_clear_method(which);
    for (auto& s : m_players) {
        clear_func(s.second);
    }
    if (which != Type::Week) {
        m_total_top.clear();
    }
    if (which != Type::Total) {
        m_week_top.clear();
    }
}

const std::vector<std::pair<std::string, unsigned>>
Scoreboard::get_top(Type which, size_t num_top) const
{
    auto getter = Scores::get_score_getter(which);
    auto& top_list = (which == Type::Week ? m_week_top : m_total_top);

    auto top = std::vector<std::pair<std::string, unsigned>>{};
    top.reserve(num_top);

    size_t count = 0;
    for (auto player = top_list.begin(); player != top_list.end();
         ++player, ++count) {
        auto score = static_cast<unsigned>(getter(m_players.at(*player)));
        top.emplace_back(*player, score);
    }
    for (; count < num_top; ++count) {
        top.emplace_back("---", 0u);
    }

    return top;
}