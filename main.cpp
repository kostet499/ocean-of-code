#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <random>
#include <sstream>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int height = 15, width = 15;
vector<vector<int> > field_mask, my_mask, enemy_mask;
int my_x, my_y;
const string move_command = "MOVE";
const string surface_command = "SURFACE";
const string torpedo_command = "TORPEDO";
const string silence_command = "SILENCE";

void and_mask(vector<vector<int> > &mask1, const vector<vector<int> > &mask2) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            mask1[i][j] &= mask2[i][j];
        }
    }
}

void or_mask(vector<vector<int> > &mask1, const vector<vector<int> > &mask2) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            mask1[i][j] |= mask2[i][j];
        }
    }
}

vector<vector<int> > get_sector_mask(int sector_index) {
    int y = (sector_index - 1) / 3, x = (sector_index - 1) % 3;
    vector<vector<int> > mask(width);
    for (int i = 0; i < width; ++i) {
        mask[i].resize(height);
        for (int j = 0; j < height; ++j) {
            int cx = i / 5, cy = j / 5;
            if (cx == x && cy == y) {
                mask[i][j] = 1;
            }
        }
    }
    return mask;
}

int sum_mask(const vector<vector<int> >& mask) {
    int sum = 0;
    for (auto el : mask) {
        for (auto e : el) {
            sum += e;
        }
    }
    return sum;
}

void shift_up(vector<vector<int> > &mask) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (j == height - 1) {
                mask[i][j] = 0;
            } else {
                mask[i][j] = mask[i][j + 1];
            }
        }
    }
    and_mask(mask, field_mask);
}

void shift_down(vector<vector<int> > &mask) {
    for (int i = 0; i < width; ++i) {
        for (int j = height - 1; j > -1; --j) {
            if (j == 0) {
                mask[i][j] = 0;
            } else {
                mask[i][j] = mask[i][j - 1];
            }
        }
    }
    and_mask(mask, field_mask);
}

void shift_right(vector<vector<int> > &mask) {
    for (int i = width - 1; i > -1; --i) {
        for (int j = 0; j < height; ++j) {
            if (i == 0) {
                mask[i][j] = 0;
            } else {
                mask[i][j] = mask[i - 1][j];
            }
        }
    }
    and_mask(mask, field_mask);
}

void shift_left(vector<vector<int> > &mask) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (i == width - 1) {
                mask[i][j] = 0;
            } else {
                mask[i][j] = mask[i + 1][j];
            }
        }
    }
    and_mask(mask, field_mask);
}

const int TORPEDO_DEPTH = 4;
pair<int, int> shoot_torpedo(int x, int y, int depth, set<pair<int, int> > &used) {
    if (used.find(make_pair(x, y)) != used.end() ||
        x < 0 || y < 0 || x == width || y == height ||
        depth > TORPEDO_DEPTH || !field_mask[x][y]) {
        return  make_pair(-1, -1);
    }

    used.insert(make_pair(x, y));
    vector<pair<int, int>> variants;
    variants.emplace_back(shoot_torpedo(x + 1, y,depth + 1,used));
    variants.emplace_back(shoot_torpedo(x - 1, y,depth + 1,used));
    variants.emplace_back(shoot_torpedo(x, y + 1,depth + 1,used));
    variants.emplace_back(shoot_torpedo(x, y - 1,depth + 1,used));
    variants.emplace_back(make_pair(x, y));
    for (auto var : variants) {
        if (var.first == -1 || !enemy_mask[var.first][var.second]) {
            continue;
        } else {
            if ((max(abs(var.first - my_x), abs(var.second - my_y)) < 2 && sum_mask(enemy_mask) > 1)) {
                continue;
            }
            return var;
        }
    }
    return make_pair(-1, -1);
}

pair<int, int> choose_begin_point() {
    for (int i = width - 1; i > -1; --i) {
        for (int j = 0; j < height; ++j) {
            if (field_mask[i][j] == 1) {
                return make_pair(i, j);
            }
        }
    }
}

void fill_torpedo_mask(int x, int y, int depth, vector<vector<int> > &mask) {
    if (x < 0 || y < 0 || x == width || y == height ||
        depth > TORPEDO_DEPTH || !field_mask[x][y] || mask[x][y]) {
        return;
    }
    mask[x][y] = 1;
    fill_torpedo_mask(x + 1, y, depth + 1, mask);
    fill_torpedo_mask(x - 1, y, depth + 1, mask);
    fill_torpedo_mask(x, y + 1, depth + 1, mask);
    fill_torpedo_mask(x, y - 1, depth + 1, mask);
}

void analyze_enemy(const string &command) {
    for (int i = 0; i < command.size(); ++i) {
        if (command[i] < 'A' || command[i] > 'Z') continue;

        if (command.find(move_command, i) == i) {
            char move = command[command.find(move_command, i) + move_command.size() + 1];
            if (move == 'N') {
                shift_up(enemy_mask);
            }
            if (move == 'S') {
                shift_down(enemy_mask);
            }
            if (move == 'W') {
                shift_left(enemy_mask);
            }
            if (move == 'E') {
                shift_right(enemy_mask);
            }
        }
        else if (command.find(surface_command, i) == i) {
            int sector_index = command[command.find(surface_command, i) + surface_command.size() + 1] - '0';
            and_mask(enemy_mask, get_sector_mask(sector_index));
        }
        else if (command.find(silence_command, i) == i) {
            vector<vector<int> > n, s, e, w;
            n = s = e = w = enemy_mask;
            for (int j = 1; j <= 4; ++j) {
                shift_left(w);
                shift_right(e);
                shift_down(s);
                shift_up(n);
                or_mask(enemy_mask, w);
                or_mask(enemy_mask, e);
                or_mask(enemy_mask, s);
                or_mask(enemy_mask, n);
            }
        }
        else if (command.find(torpedo_command, i) == i) {
            stringstream in(command.substr(command.find(' ', i) + 1, string::npos));
            int x, y;
            in >> x >> y;
            auto torpedo_mask = get_sector_mask(-1);
            fill_torpedo_mask(x, y, 0, torpedo_mask);
            and_mask(enemy_mask, torpedo_mask);
        }

        while(i < command.size() && command[i] >= 'A' && command[i] <= 'Z') ++i;
    }
    if (sum_mask(enemy_mask) == 1) {
        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                if (enemy_mask[i][j]) {
                    cout << "MSG " << i << " " << j << "|";
                }
            }
        }
    } else {
        cout << "MSG " << sum_mask(enemy_mask) << "|";
    }
}

vector<int> check_move_variant(int x, int y, vector<vector<int> > &cur_mask) {
    if (x < 0 || y < 0 || x == width || y == height || !field_mask[x][y] || !cur_mask[x][y]) {
        return {0, -1, -1};
    }
    int cur_count = 1;
    cur_mask[x][y] = 0;
    cur_count += check_move_variant(x + 1, y, cur_mask)[0];
    cur_count += check_move_variant(x - 1, y, cur_mask)[0];
    cur_count += check_move_variant(x, y + 1, cur_mask)[0];
    cur_count += check_move_variant(x, y - 1, cur_mask)[0];
    return {cur_count, x, y};
}

char select_move() {
    vector<pair<vector<int>, char> > variants;
    auto cur_mask = my_mask;
    variants.emplace_back(make_pair(check_move_variant(my_x + 1, my_y, cur_mask), 'E'));
    cur_mask = my_mask;
    variants.emplace_back(make_pair(check_move_variant(my_x - 1, my_y, cur_mask), 'W'));
    cur_mask = my_mask;
    variants.emplace_back(make_pair(check_move_variant(my_x, my_y + 1, cur_mask), 'S'));
    cur_mask = my_mask;
    variants.emplace_back(make_pair(check_move_variant(my_x, my_y - 1, cur_mask), 'N'));
    sort(variants.begin(), variants.end(), greater<>());

    vector<pair<vector<int>, char> > pool;
    for (auto var : variants) {
        if (!pool.empty() && pool.back().first[0] > var.first[0]) {
            break;
        }
        if (var.first[0]) {
            pool.emplace_back(var);
        }
    }
    if (pool.empty()) {
        return '?';
    }
    auto pos = pool[rand() % pool.size()];
    my_x = pos.first[1];
    my_y = pos.first[2];
    my_mask[my_x][my_y] = 0;
    return pos.second;
}

string make_my_command(int torpedoCooldown, int silenceCooldown) {
    string power = torpedo_command, move = move_command, step;

    if (torpedoCooldown == 0 && sum_mask(enemy_mask) < 20) {
        set<pair<int, int> > used;
        auto possible_shoot = shoot_torpedo(my_x, my_y, 0, used);
        if (possible_shoot.first != -1) {
            cout << torpedo_command + " " + to_string(possible_shoot.first) + " " + to_string(possible_shoot.second)
                 << "|";
        }
    }

    if (torpedoCooldown == 0) {
        power = silence_command;
    }

    if (silenceCooldown == 0) {
        move = silence_command;
        power = "1";
    }

    char move_direction = select_move();
    if (move_direction == '?') {
        my_mask = field_mask;
        my_mask[my_x][my_y] = 0;
        return surface_command;
    }
    return move + " " + move_direction + " " + step + power;
}

int main()
{

    int myId;
    cin >> width >> height >> myId; cin.ignore();

    field_mask = get_sector_mask(-1);
    for (int j = 0; j < height; j++) {
        string line;
        getline(cin, line);
        for (int i = 0; i < width; ++i) {
            field_mask[i][j] = line[i] == '.' ? 1 : 0;
        }
    }
    my_mask = enemy_mask = field_mask;

    auto begin_point = choose_begin_point();
    my_x = begin_point.first, my_y = begin_point.second;
    my_mask[my_x][my_y] = 0;
    cout << my_x << " " << my_y << endl;

    // game loop
    int ppp = 1;
    while (ppp) {
        if (my_x == -100000) ppp = 0; // fiction

        int myLife;
        int oppLife;
        int torpedoCooldown;
        int sonarCooldown;
        int silenceCooldown;
        int mineCooldown;
        cin >> my_x >> my_y >> myLife >> oppLife >> torpedoCooldown >> sonarCooldown >> silenceCooldown >> mineCooldown; cin.ignore();
        string sonarResult;
        cin >> sonarResult; cin.ignore();
        string opponentOrders;
        getline(cin, opponentOrders);
        analyze_enemy(opponentOrders);
        cout << make_my_command(torpedoCooldown, silenceCooldown) << endl;
    }
}