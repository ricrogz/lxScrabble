//
// Created by invik on 15/09/19.
//

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

const size_t TOP_SIZE = 10;

class Scoreboard
{
  public:
    enum class Type { Week, Total, All };

    class Scores
    {
      public:
        static std::function<void(Scores&)> get_clear_method(Type which);
        static std::function<unsigned long(const Scores&)>
        get_score_getter(Type which);

        Scores() = default;
        Scores(unsigned long week, unsigned long total);
        void add(unsigned long score);

      private:
        unsigned long m_week{0UL};
        unsigned long m_total{0UL};

        void clear_week();
        void clear_total();
        void clear_all();
        unsigned long get_week() const;
        unsigned long get_total() const;
    };

    static Scoreboard* read_scoreboard(const std::string& scores_file);

    Scoreboard() = delete;
    Scoreboard(const Scoreboard&) = delete;
    Scoreboard(Scoreboard&&) = delete;
    Scoreboard& operator=(const Scoreboard&) = delete;
    Scoreboard& operator=(Scoreboard&&) = delete;
    ~Scoreboard() = default;

    void add_score(const std::string& player, unsigned long score);
    unsigned long get_score(const std::string& player, Type which) const;
    void save() const;
    void clear(Type which);

    const std::vector<std::pair<std::string, unsigned>>
    get_top(Type which, size_t num_top = TOP_SIZE) const;

  private:
    std::string m_scorefile;
    std::unordered_map<std::string, Scores> m_players;
    std::vector<std::string> m_week_top;
    std::vector<std::string> m_total_top;

    explicit Scoreboard(std::string scorefile);

    void update_top(std::vector<std::string>& top, Type which,
                    const std::string& player);
};
